Запуск:
mkdir <build-dir>
cd <build-dir>
cmake <source-dir>
make
cd <source-dir>
chmod +x install.sh
chmod +x uninstall.sh
./install.sh <build-dir> <port> <threads-count>
sudo systemctl <start|restart|stop|status> RatingService
./uninstall.sh
