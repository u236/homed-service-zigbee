FROM    127.0.0.1:5000/homed-service:latest
ARG     TARGETARCH
RUN     mkdir /etc/homed
RUN     ln -s /data/homed-zigbee.conf /etc/homed/homed-zigbee.conf
RUN     ln -s /data/homed.log /var/log/homed.log
ADD     share /usr/share
COPY    homed-zigbee-$TARGETARCH /usr/bin/homed-zigbee
CMD     ["/usr/bin/homed-zigbee"]
