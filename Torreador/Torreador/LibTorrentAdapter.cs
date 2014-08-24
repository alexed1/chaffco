using System;
using System.Runtime.InteropServices;

namespace Torreador
{
    /// <summary>
    /// Adapts the libtorrent interface.
    /// </summary>
    public class LibTorrentAdapter : IDisposable
    {
        [DllImport("LibTorrentAdapter.dll", EntryPoint = "Initialize")]
        private static extern Int32 initialize([MarshalAs(UnmanagedType.LPStr)]String address, UInt16 port);

        /// <summary>
        /// Initializes libtorrent.
        /// </summary>
        /// <param name="address">The IP address to listen on.</param>
        /// <param name="port">The port number to listen on.</param>
        /// <exception cref="LibTorrentAdapterException">Failed to initialize libtorrent.</exception>
        public void Initialize(String address,
            UInt16 port)
        {
            if (initialize(address, port) != 0)
            {
                throw new LibTorrentAdapterException("Failed to initialize libtorrent.");
            }

            IsInitialized = true;
        }

        /// <summary>
        /// Gets a value indicating whether libtorrent is initialized.
        /// </summary>
        public Boolean IsInitialized
        {
            get;
            private set;
        }

        [DllImport("LibTorrentAdapter.dll", EntryPoint = "UnInitialize")]
        private static extern void uninitialize();

        /// <summary>
        /// Uninitializes libtorrent.
        /// </summary>
        public void UnInitialize()
        {
            if (IsInitialized)
            {
                uninitialize();
                IsInitialized = false;
            }
        }

        [DllImport("LibTorrentAdapter.dll", EntryPoint = "AddTorrent")]
        private static extern Int32 addtorrent([MarshalAs(UnmanagedType.LPStr)]String url,
            [MarshalAs(UnmanagedType.LPStr)]String path);

        /// <summary>
        /// Adds a torrent to download.
        /// </summary>
        /// <param name="torrentUrl">The url to download a torrent file from.</param>
        /// <param name="destinationPath">The path to download the torrent content to.</param>
        /// <exception cref="InvalidOperationException">libtorrent is not initialized.</exception>
        /// <exception cref="LibTorrentAdapterException">Failed to add a torrent.</exception>
        public void AddTorrent(String torrentUrl,
            String destinationPath)
        {
            if (!IsInitialized)
            {
                throw new InvalidOperationException("libtorrent is not initialized.");
            }

            if (addtorrent(torrentUrl, destinationPath) != 0)
            {
                throw new LibTorrentAdapterException("Failed to add a torrent.");
            }
        }

        [DllImport("LibTorrentAdapter.dll", EntryPoint = "Pause")]
        private static extern Int32 pause();

        /// <summary>
        /// Pauses libtorrent.
        /// </summary>
        /// <exception cref="InvalidOperationException">libtorrent is not initialized.</exception>
        /// <exception cref="LibTorrentAdapterException">Failed to pause libtorrent.</exception>
        public void Pause()
        {
            if (!IsInitialized)
            {
                throw new InvalidOperationException("libtorrent is not initialized.");
            }

            if (pause() != 0)
            {
                throw new LibTorrentAdapterException("Failed to pause libtorrent.");
            }
        }

        [DllImport("LibTorrentAdapter.dll", EntryPoint = "Resume")]
        private static extern Int32 resume();

        /// <summary>
        /// Resumes libtorrent.
        /// </summary>
        /// <exception cref="InvalidOperationException">libtorrent is not initialized.</exception>
        /// <exception cref="LibTorrentAdapterException">Failed to resume libtorrent.</exception>
        public void Resume()
        {
            if (!IsInitialized)
            {
                throw new InvalidOperationException("libtorrent is not initialized.");
            }

            if (resume() != 0)
            {
                throw new LibTorrentAdapterException("Failed to resume libtorrent.");
            }
        }

        public void Dispose()
        {
            UnInitialize();
            GC.SuppressFinalize(this);
        }

        ~LibTorrentAdapter()
        {
            UnInitialize();
        }
    }

    public class LibTorrentAdapterException : Exception
    {
        public LibTorrentAdapterException(string message)
            : base(message)
        {
        }
    }
}
