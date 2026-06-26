FROM debian:bookworm

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get upgrade -y && \
	apt-get install curl net-tools iproute2 openssh-server nginx tor -y && \
	rm -rf /var/lib/apt/lists/*

COPY ./srcs/nginx.conf /etc/nginx/nginx.conf
COPY ./srcs/index.html /var/www/html/index.html

CMD ["sleep", "infinity"]
