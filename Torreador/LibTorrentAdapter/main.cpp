#include <cstdint>

#define BOOST_ASIO_SEPARATE_COMPILATION
#define TORRENT_NO_DEPRECATE

#include "LibTorrentAdapter.h"

#define LIBTORRENT_ADAPTER_API __declspec(dllexport)

using namespace std;

/// <summary>
/// Initializes libtorrent.
/// </summary>
/// <param name="address">The IP address to listen on.</param>
/// <param name="port">The port number to listen on.</param>
/// <returns>If the function succeeds, the return value is zero.</returns>
LIBTORRENT_ADAPTER_API int32_t __stdcall Initialize(const char* addressToListenOn,
	uint16_t portToListenOn)
{
	return LibTorrentAdapter::Instance().Initialize(addressToListenOn, portToListenOn);
}

/// <summary>
/// Adds a torrent to download.
/// </summary>
/// <param name="torrentUrl">The url to download a torrent file from.</param>
/// <param name="destinationPath">The path to download the torrent content to.</param>
/// <returns>If the function succeeds, the return value is zero.</returns>
LIBTORRENT_ADAPTER_API int32_t __stdcall AddTorrent(const char *torrentUrl,
	const char *destinationPath)
{
	return LibTorrentAdapter::Instance().AddTorrent(torrentUrl, destinationPath);
}

/// <summary>
/// Pauses libtorrent.
/// </summary>
/// <returns>If the function succeeds, the return value is zero.</returns>
LIBTORRENT_ADAPTER_API int32_t __stdcall Pause()
{
	return LibTorrentAdapter::Instance().Pause();
}

/// <summary>
/// Resumes libtorrent.
/// </summary>
/// <returns>If the function succeeds, the return value is zero.</returns>
LIBTORRENT_ADAPTER_API int32_t __stdcall Resume()
{
	return LibTorrentAdapter::Instance().Resume();
}

/// <summary>
/// Uninitializes libtorrent.
/// </summary>
LIBTORRENT_ADAPTER_API void __stdcall UnInitialize()
{
	LibTorrentAdapter::Instance().UnInitialize();
}