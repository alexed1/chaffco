using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows.Forms;
using System.Windows.Input;

namespace Torreador
{
    public class MainWindowViewModel : INotifyPropertyChanged
    {
        private LibTorrentAdapter _adapter;

        public MainWindowViewModel(LibTorrentAdapter adapter)
        {
            if (adapter == null)
            {
                throw new ArgumentNullException("adapter");
            }

            _adapter = adapter;

            IpAddress = "0.0.0.0";
            IpPort = 6881;
            AreSettingsEnabled = true;

            EnterUrlExampleCommand = new RelayCommand(ExecuteEnterUrlExampleCommand);
            DownloadToCommand = new RelayCommand(ExecuteDownloadToCommand,
                CanExecuteDownloadToCommand);
            PauseCommand = new RelayCommand(ExecutePauseCommand, CanExecutePauseCommand);
            ResumeCommand = new RelayCommand(ExecuteResumeCommand, CanExecuteResumeCommand);
        }

        public ICommand DownloadToCommand
        {
            get;
            private set;
        }

        private void ExecuteDownloadToCommand(object arg)
        {
            using (var dlg = new FolderBrowserDialog())
            {
                if (dlg.ShowDialog() == DialogResult.OK)
                {
                    if (!_adapter.IsInitialized)
                    {
                        _adapter.Initialize(IpAddress, IpPort);

                        AreSettingsEnabled = false;
                    }

                    _adapter.AddTorrent(TorrentUrl, dlg.SelectedPath);

                    TorrentUrl = string.Empty;

                    IsPauseVisible = true;
                }
            }
        }
                
        private bool CanExecuteDownloadToCommand(object arg)
        {
            return IsValidUrl(TorrentUrl);
        }        

        public ICommand PauseCommand
        {
            get;
            private set;
        }

        private Boolean _isPaused;

        private void ExecutePauseCommand(object arg)
        {
            _adapter.Pause();

            _isPaused = true;
            IsPauseVisible = false;
            IsResumeVisible = true;
        }

        private bool CanExecutePauseCommand(object arg)
        {
            return _adapter.IsInitialized && !_isPaused;
        }

        public ICommand ResumeCommand
        {
            get;
            private set;
        }

        private void ExecuteResumeCommand(object arg)
        {
            _adapter.Resume();

            _isPaused = false;
            IsPauseVisible = true;
            IsResumeVisible = false;
        }

        private bool CanExecuteResumeCommand(object arg)
        {
            return _adapter.IsInitialized && _isPaused;
        }

        public ICommand EnterUrlExampleCommand
        {
            get;
            private set;
        }

        private void ExecuteEnterUrlExampleCommand(object arg)
        {
            TorrentUrl = @"http://torcache.net/torrent/D5976A4FC7C3A5F745CA9E77B718F09B520C3166.torrent?title=[kickass.to]maleficent.2014.dvdrip.xvid.maxspeed";
        }
        
        static bool IsValidUrl(string urlString)
        {
            Uri uri;
            return Uri.TryCreate(urlString, UriKind.Absolute, out uri)
                && (uri.Scheme == Uri.UriSchemeHttp
                || uri.Scheme == Uri.UriSchemeHttps);
        }

        String _torrentUrl;
        public String TorrentUrl
        {
            get { return _torrentUrl; }
            set
            {
                if (_torrentUrl != value)
                {
                    _torrentUrl = value;
                    RaisePropertyChanged();
                }
            }
        }

        String _ipAddress;
        public String IpAddress
        {
            get { return _ipAddress; }
            set
            {
                if (_ipAddress != value)
                {
                    _ipAddress = value;
                    RaisePropertyChanged();
                }
            }
        }

        UInt16 _ipPort;
        public UInt16 IpPort
        {
            get { return _ipPort; }
            set
            {
                if (_ipPort != value)
                {
                    _ipPort = value;
                    RaisePropertyChanged();
                }
            }
        }

        private Boolean _isPauseVisible;
        public Boolean IsPauseVisible
        {
            get { return _isPauseVisible; }
            set
            {
                if (_isPauseVisible != value)
                {
                    _isPauseVisible = value;
                    RaisePropertyChanged();
                }
            }
        }

        private Boolean _isResumeVisible;
        public Boolean IsResumeVisible
        {
            get { return _isResumeVisible; }
            set
            {
                if (_isResumeVisible != value)
                {
                    _isResumeVisible = value;
                    RaisePropertyChanged();
                }
            }
        }

        private Boolean _areSettingsEnabled;
        public Boolean AreSettingsEnabled
        {
            get { return _areSettingsEnabled; }
            set
            {
                if (_areSettingsEnabled != value)
                {
                    _areSettingsEnabled = value;
                    RaisePropertyChanged();
                }
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        private void RaisePropertyChanged([CallerMemberName]string propertyName = null)
        {
            var handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(propertyName));
            }
        }
    }
}
