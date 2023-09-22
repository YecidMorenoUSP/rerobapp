# syntax=docker/dockerfile:1

FROM debian:11.7-slim

ENV TZ=America/Sao_Paulo
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && apt-get install -y \
	sudo\
	git\
	build-essential\
	curl\
	wget

RUN apt-get install -y \
	cmake\
	g++-arm-linux-gnueabihf

RUN apt-get install -y \
	linux-image-5.10.0-25-rt-amd64

RUN echo 'root ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
RUN echo 'yecid ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

USER rerob
