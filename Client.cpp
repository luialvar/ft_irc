// Client.cpp
#include "Client.hpp"   // -> declaracion de la clase Client

Client::Client()
	: _fd(-1), _ip(""), _nickname(""), _username(""), _realname(""),
	  _recvBuffer(""), _sendBuffer(""), _passOk(false), _registered(false)
{
}

Client::Client(int fd, const std::string& ip)
	: _fd(fd), _ip(ip), _nickname(""), _username(""), _realname(""),
	  _recvBuffer(""), _sendBuffer(""), _passOk(false), _registered(false)
{
}

Client::~Client()
{
}

int Client::getFd() const
{
	return _fd;
}

const std::string& Client::getIp() const
{
	return _ip;
}

const std::string& Client::getNickname() const
{
	return _nickname;
}

const std::string& Client::getUsername() const
{
	return _username;
}

const std::string& Client::getRealname() const
{
	return _realname;
}

const std::string& Client::getRecvBuffer() const
{
	return _recvBuffer;
}

//luialvar
const std::string& Client::getSendBuffer() const
{
	return _sendBuffer;
}
//luialvar

bool Client::hasPassedPassword() const
{
	return _passOk;
}

bool Client::isRegistered() const
{
	return _registered;
}

void Client::setFd(int fd)
{
	_fd = fd;
}

void Client::setIp(const std::string& ip)
{
	_ip = ip;
}

void Client::setNickname(const std::string& nickname)
{
	_nickname = nickname;
}

void Client::setUsername(const std::string& username)
{
	_username = username;
}

void Client::setRealname(const std::string& realname)
{
	_realname = realname;
}

void Client::setPassedPassword(bool value)
{
	_passOk = value;
}

void Client::setRegistered(bool value)
{
	_registered = value;
}

void Client::appendToBuffer(const std::string& data)
{
	_recvBuffer += data;
}

void Client::clearBuffer()
{
	_recvBuffer.clear();
}

void Client::eraseFromBuffer(std::string::size_type count)
{
	if (count >= _recvBuffer.size())
	{
		_recvBuffer.clear();
		return;
	}
	_recvBuffer.erase(0, count);
}


void Client::appendToSendBuffer(const std::string& data)
{
	_sendBuffer += data;
}

void Client::eraseFromSendBuffer(std::string::size_type count)
{
	if (count >= _sendBuffer.size())
	{
		_sendBuffer.clear();
		return;
	}
	_sendBuffer.erase(0, count);
}

