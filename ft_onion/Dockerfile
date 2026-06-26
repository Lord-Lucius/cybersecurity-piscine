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
RUN chown -R debian-tor:debian-tor /var/lib/tor/hidden_service && chmod 700 /var/lib/tor/hidden_service

COPY ./tools/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

CMD ["/entrypoint.sh"]
