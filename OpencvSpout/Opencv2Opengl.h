#pragma once
#include "opencv2\core.hpp"
#include "Glew\glew.h"
#include "GL\freeglut.h"
#include "Spout.h"
#include "SpoutDLL.h"
class Opencv2Spout
{
public:
	Opencv2Spout(int argc, char **argv, unsigned int width, unsigned int height, bool forceDX9 = false);
	~Opencv2Spout();
	static GLuint matToTexture(cv::Mat &mat, GLenum minFilter, GLenum magFilter, GLenum wrapFilter);
	void draw(cv::Mat &camFrame,bool drawImage);
	bool initReceiver(char* name);
	cv::Mat receiveTexture();
private:
	bool m_bReceiverCreated;
	unsigned int m_iWidth, m_iHeight;
	char* m_receiverName;
	SpoutSender* spout;
	SpoutReceiver* spoutReceiver;
};

