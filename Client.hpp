// Client.hpp
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>   // -> std::string

class Client
{
private:
	int			_fd;
	std::string	_ip;
	std::string	_nickname;
	std::string	_username;
	std::string	_realname;
	std::string	_recvBuffer;
	bool		_passOk;
	bool		_registered;

public:
	Client();
	Client(int fd, const std::string& ip);
	~Client();

	int getFd() const;
	const std::string& getIp() const;
	const std::string& getNickname() const;
	const std::string& getUsername() const;
	const std::string& getRealname() const;
	const std::string& getRecvBuffer() const;
	bool hasPassedPassword() const;
	bool isRegistered() const;

	void setFd(int fd);
	void setIp(const std::string& ip);
	void setNickname(const std::string& nickname);
	void setUsername(const std::string& username);
	void setRealname(const std::string& realname);
	void setPassedPassword(bool value);
	void setRegistered(bool value);

	void appendToBuffer(const std::string& data);
	void clearBuffer();
	void eraseFromBuffer(std::string::size_type count);
};

#endif