#define BOOST_ASIO_SEPARATE_COMPILATION
#define TORRENT_NO_DEPRECATE

#include "LibTorrentAdapter.h"

#pragma warning(push)
#pragma warning(disable : 4127)

#include "libtorrent/bencode.hpp"
#include "libtorrent/alert_types.hpp"

#pragma warning(pop) 

using namespace std;
using namespace libtorrent;

int LibTorrentAdapter::load_file(std::string const& filename,
	std::vector<char>& v,
	libtorrent::error_code& ec,
	int32_t limit)
{
	ec.clear();
	FILE* f = NULL;
	fopen_s(&f, filename.c_str(), "rb");
	if (f == NULL)
	{
		ec.assign(errno, boost::system::get_generic_category());
		return -1;
	}

	int r = fseek(f, 0, SEEK_END);
	if (r != 0)
	{
		ec.assign(errno, boost::system::get_generic_category());
		fclose(f);
		return -1;
	}
	long s = ftell(f);
	if (s < 0)
	{
		ec.assign(errno, boost::system::get_generic_category());
		fclose(f);
		return -1;
	}

	if (s > limit)
	{
		fclose(f);
		return -2;
	}

	r = fseek(f, 0, SEEK_SET);
	if (r != 0)
	{
		ec.assign(errno, boost::system::get_generic_category());
		fclose(f);
		return -1;
	}

	v.resize(s);
	if (s == 0)
	{
		fclose(f);
		return 0;
	}

	r = fread(&v[0], 1, v.size(), f);
	if (r < 0)
	{
		ec.assign(errno, boost::system::get_generic_category());
		fclose(f);
		return -1;
	}

	fclose(f);

	if (r != s) return -3;

	return 0;
}

int LibTorrentAdapter::save_file(std::string const& filename, std::vector<char>& v)
{
	using namespace libtorrent;

	// TODO: don't use internal file type here. use fopen()
	file f;
	libtorrent::error_code ec;
	if (!f.open(filename, file::write_only, ec)) return -1;
	if (ec) return -1;
	file::iovec_t b = { &v[0], v.size() };
	size_type written = f.writev(0, &b, 1, ec);
	if (written != int(v.size())) return -3;
	if (ec) return -3;
	return 0;
}

unique_ptr<LibTorrentAdapter> LibTorrentAdapter::instancePtr_ = nullptr;

LibTorrentAdapter& LibTorrentAdapter::Instance()
{
	if (instancePtr_ == nullptr)
	{
		instancePtr_.reset(new LibTorrentAdapter);
	}

	return *instancePtr_;
}

int32_t LibTorrentAdapter::Initialize(const char* addressToListenOn,
	uint16_t portToListenOn)
{
	sessionPtr_.reset(new session(fingerprint("LT", LIBTORRENT_VERSION_MAJOR, LIBTORRENT_VERSION_MINOR, 0, 0)
		, session::add_default_plugins
		, alert::all_categories
		& ~(alert::dht_notification
		+ alert::progress_notification
		+ alert::debug_notification
		+ alert::stats_notification)));

	std::vector<char> in;
	libtorrent::error_code error;
	if (load_file(".ses_state", in, error) == 0)
	{
		lazy_entry e;
		if (lazy_bdecode(&in[0], &in[0] + in.size(), e, error) == 0)
			sessionPtr_->load_state(e);
	}

#ifndef TORRENT_DISABLE_GEO_IP
	sessionPtr_->load_asnum_db("GeoIPASNum.dat");
	sessionPtr_->load_country_db("GeoIP.dat");
#endif			

	bool start_dht = true;
	bool start_upnp = true;
	bool start_lsd = true;

	if (start_lsd)
		sessionPtr_->start_lsd();

	if (start_upnp)
	{
		sessionPtr_->start_upnp();
		sessionPtr_->start_natpmp();
	}
		
	proxy_settings ps;
	sessionPtr_->set_proxy(ps);

	sessionPtr_->listen_on(std::make_pair(portToListenOn, portToListenOn)
		, error, addressToListenOn);
	if (error)
	{
		//fprintf(stderr, "failed to listen%s%s on ports %d-%d: %s\n"
		//	, bind_to_interface.empty() ? "" : " on ", bind_to_interface.c_str()
		//	, listen_port, listen_port + 1, ec.message().c_str());
		return 1;
	}

	session_settings settings;

#ifndef TORRENT_DISABLE_DHT
	dht_settings dht;
	dht.privacy_lookups = true;
	sessionPtr_->set_dht_settings(dht);

	if (start_dht)
	{
		settings.use_dht_as_fallback = false;

		sessionPtr_->add_dht_router(std::make_pair(
			std::string("router.bittorrent.com"), 6881));
		sessionPtr_->add_dht_router(std::make_pair(
			std::string("router.utorrent.com"), 6881));
		sessionPtr_->add_dht_router(std::make_pair(
			std::string("router.bitcomet.com"), 6881));

		sessionPtr_->start_dht();
	}
#endif

	settings.user_agent = "client_test/" LIBTORRENT_VERSION;
	settings.choking_algorithm = session_settings::auto_expand_choker;
	settings.disk_cache_algorithm = session_settings::avoid_readback;
	settings.volatile_read_cache = false;

	sessionPtr_->set_settings(settings);
		
	return 0;
}

void LibTorrentAdapter::UnInitialize()
{
	BOOST_ASSERT(sessionPtr_ != nullptr);

	sessionPtr_->pause();

	int32_t num_outstanding_resume_data = 0;
	std::vector<torrent_status> temp;
	sessionPtr_->get_torrent_status(&temp, [](libtorrent::torrent_status const&){ return true; }, 0);
	for (std::vector<torrent_status>::iterator i = temp.begin();
		i != temp.end(); ++i)
	{
		torrent_status& st = *i;
		if (!st.handle.is_valid())
		{
			continue;
		}

		if (!st.has_metadata)
		{
			continue;
		}

		if (!st.need_save_resume)
		{
			continue;
		}

		// save_resume_data will generate an alert when it's done
		st.handle.save_resume_data();
		++num_outstanding_resume_data;
	}

	int32_t num_paused = 0, num_failed = 0;
	while (num_outstanding_resume_data > 0)
	{
		alert const* a = sessionPtr_->wait_for_alert(seconds(10));
		if (a == 0) continue;

		std::deque<alert*> alerts;
		sessionPtr_->pop_alerts(&alerts);

		for (std::deque<alert*>::iterator i = alerts.begin()
			, end(alerts.end()); i != end; ++i)
		{
			// make sure to delete each alert
			std::auto_ptr<alert> a(*i);

			torrent_paused_alert const* tp = alert_cast<torrent_paused_alert>(*i);
			if (tp)
			{
				++num_paused;
				continue;
			}

			if (alert_cast<save_resume_data_failed_alert>(*i))
			{
				++num_failed;
				--num_outstanding_resume_data;
				continue;
			}

			save_resume_data_alert const* rd = alert_cast<save_resume_data_alert>(*i);
			if (!rd) continue;
			--num_outstanding_resume_data;

			if (!rd->resume_data) continue;

			torrent_handle h = rd->handle;
			torrent_status st = h.status(torrent_handle::query_save_path);
			std::vector<char> out;
			bencode(std::back_inserter(out), *rd->resume_data);
			save_file(combine_path(st.save_path, combine_path(".resume", to_hex(st.info_hash.to_string()) + ".resume")), out);
		}
	}

	entry session_state;
	sessionPtr_->save_state(session_state);

	std::vector<char> out;
	bencode(std::back_inserter(out), session_state);
	save_file(".ses_state", out);

	sessionPtr_.reset(nullptr);
}

int32_t LibTorrentAdapter::AddTorrent(const char *torrentUrl,
	const char *destination)
{
	BOOST_ASSERT(sessionPtr_ != nullptr);

	if (sessionPtr_ == nullptr)
	{
		return 1;
	}

	libtorrent::error_code error;
	// create directory for resume files
	// TODO: don't use internal create_directory function
	create_directory(combine_path(destination, ".resume"), error);
	if (error)
	{
		//fprintf(stderr, "failed to create resume file directory: %s\n", ec.message().c_str());
		return 1;
	}

	add_torrent_params p;
	p.save_path = destination;
	p.storage_mode = storage_mode_sparse;
	p.url = torrentUrl;

	sessionPtr_->async_add_torrent(p);

	return 0;
}

int32_t LibTorrentAdapter::Pause()
{
	BOOST_ASSERT(sessionPtr_ != nullptr);

	if (sessionPtr_ == nullptr)
	{
		return 1;
	}

	sessionPtr_->pause();

	return 0;
}

int32_t LibTorrentAdapter::Resume()
{
	BOOST_ASSERT(sessionPtr_ != nullptr);

	if (sessionPtr_ == nullptr)
	{
		return 1;
	}

	sessionPtr_->resume();

	return 0;
}