using System.Windows;

namespace Torreador
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        protected override void OnStartup(StartupEventArgs e)
        {
            base.OnStartup(e);

            DispatcherUnhandledException += (s, a) => { MessageBox.Show(a.Exception.Message,
                "Error", MessageBoxButton.OK, MessageBoxImage.Error); }; 

            LibTorrentAdapter adapter = new LibTorrentAdapter();
            var viewModel = new MainWindowViewModel(adapter);

            MainWindow window = new MainWindow();
            window.DataContext = viewModel;
            window.Closed += (s, a) => { adapter.Dispose(); };
            window.Show();
        }
    }
}
