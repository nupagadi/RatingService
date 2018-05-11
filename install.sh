if [ $# -ne 3 ]
  then
    echo "./install.sh <path/to/build> <port> <threads count>"
    exit 1
fi

FILE=$(pwd)/temp.tmp

rm -f $FILE

cd $1

TEXT="[Unit]
Description=Rating Service

[Service]
ExecStart=$(pwd)/RatingService $2 $3

[Install]
WantedBy=multi-user.target"

echo -e "$TEXT" >> $FILE

sudo mv $FILE /etc/systemd/system/RatingService.service

sudo systemctl daemon-reload
