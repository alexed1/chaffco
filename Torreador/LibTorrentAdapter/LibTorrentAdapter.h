#include <memory>
#include <string>

#include <boost\noncopyable.hpp>

#pragma warning(push)
#pragma warning(disable : 4127 4244 4245 4512)

#include "libtorrent/session.hpp"

#pragma warning(pop) 

/// <summary>
/// Adapts the libtorrent interface.
/// </summary>
class LibTorrentAdapter : boost::noncopyable
{
public:
	static LibTorrentAdapter& Instance();

	/// <summary>
	/// Initializes libtorrent.
	/// </summary>
	/// <param name="address">The IP address to listen on.</param>
	/// <param name="port">The port number to listen on.</param>
	/// <returns>If the function succeeds, the return value is zero.</returns>
	std::int32_t Initialize(const char* addressToListenOn,
		std::uint16_t portToListenOn);

	/// <summary>
	/// Adds a torrent to download.
	/// </summary>
	/// <param name="torrentUrl">The url to download a torrent file from.</param>
	/// <param name="destinationPath">The path to download the torrent content to.</param>
	/// <returns>If the function succeeds, the return value is zero.</returns>
	std::int32_t AddTorrent(const char *torrentUrl,
		const char *destinationPath);

	/// <summary>
	/// Pauses libtorrent.
	/// </summary>
	/// <returns>If the function succeeds, the return value is zero.</returns>
	std::int32_t Pause();

	/// <summary>
	/// Resumes libtorrent.
	/// </summary>
	/// <returns>If the function succeeds, the return value is zero.</returns>
	std::int32_t Resume();

	/// <summary>
	/// Uninitializes libtorrent.
	/// </summary>
	void UnInitialize();

private:
	LibTorrentAdapter() {}

	std::unique_ptr<libtorrent::session> sessionPtr_;

	static std::unique_ptr<LibTorrentAdapter> instancePtr_;

	int save_file(std::string const& filename,
		std::vector<char>& v);
	int load_file(std::string const& filename,
		std::vector<char>& v,
		libtorrent::error_code& ec,
		std::int32_t limit = 8000000);
};


