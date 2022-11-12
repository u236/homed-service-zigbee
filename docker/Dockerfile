FROM    ubuntu:latest
ARG     TARGETARCH
RUN     mkdir /etc/homed
RUN     mkdir /usr/share/homed
RUN     ln -s /data/homed-zigbee.conf /etc/homed/homed-zigbee.conf
RUN     ln -s /data/homed.log /var/log/homed.log
COPY    homed-zigbee-$TARGETARCH /usr/local/bin/homed-zigbee
COPY    zigbee.json /usr/share/homed/zigbee.json
CMD     /usr/local/bin/homed-zigbee
