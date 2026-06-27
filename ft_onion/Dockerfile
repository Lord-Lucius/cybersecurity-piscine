FROM debian:bookworm

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get upgrade -y && \
	apt-get install curl net-tools iproute2 openssh-server nginx tor -y && \
	rm -rf /var/lib/apt/lists/*

COPY ./srcs/nginx.conf /etc/nginx/nginx.conf
COPY ./srcs/index.html /var/www/html/index.html
COPY ./srcs/sshd_config /etc/ssh/sshd_config
COPY ./srcs/torrc /etc/tor/torrc

RUN mkdir -p /run/sshd
RUN mkdir -p /var/lib/tor/hidden_service
RUN chown -R debian-tor:debian-tor /var/lib/tor/hidden_service
RUN chmod 700 /var/lib/tor/hidden_service

COPY ./tools/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

RUN useradd -m -s /bin/bash ghost
RUN passwd -d ghost
RUN chmod 750 /home/ghost

RUN mkdir -p /home/ghost/.ssh

COPY ./srcs/id_ed25519.pub /home/ghost/.ssh/authorized_keys

RUN chown -R ghost:ghost /home/ghost && \
    chmod 700 /home/ghost/.ssh && \
    chmod 600 /home/ghost/.ssh/authorized_keys

CMD ["/entrypoint.sh"]
