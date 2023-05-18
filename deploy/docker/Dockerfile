FROM    127.0.0.1:5000/homed-service:latest
ARG     TARGETARCH
RUN     mkdir /etc/homed
RUN     ln -s /data/homed-zigbee.conf /etc/homed/homed-zigbee.conf
RUN     ln -s /data/homed.log /var/log/homed.log
COPY    homed-zigbee-$TARGETARCH /usr/local/homed-zigbee
ADD     library /usr/share/homed-zigbee
CMD     ["/usr/local/homed-zigbee"]
