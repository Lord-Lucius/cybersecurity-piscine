#!/bin/bash

nginx &
/usr/sbin/sshd &
su -s /bin/bash debian-tor -c "tor"
