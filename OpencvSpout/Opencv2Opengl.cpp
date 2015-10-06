#include "stdafx.h"
#include "Opencv2Opengl.h"
#include <iostream>
#include "opencv2/opencv.hpp"
using namespace std;
using namespace cv;

Opencv2Spout::Opencv2Spout(int argc, char **argv, unsigned int width, unsigned int height,bool forceDX9)
{
	m_iWidth = width;
	m_iHeight = height;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1, 1);
	glutCreateWindow("OpenGL First Window");

	glewInit();
	printf("OpenGL version supported by this platform (%s): \n", glGetString(GL_VERSION));
	 
	spout = new SpoutSender();
	if (!spout->CreateSender("Opencv2Spout", width, height,forceDX9))
	{
		int lastError = GetLastError();
		switch (lastError)
		{ 
		case ERROR_OPEN_FAILED:
			cout << "Error open failed";
				break;
		case ERROR_NOT_SUPPORTED:
			cout << "Error DX not supported by graphic card";
				break;
		default:
			cout << "last error " << lastError;
		}
		exit(0);
	}
}
bool Opencv2Spout::initReceiver(char* name)
{
	m_receiverName = name;
	spoutReceiver = new SpoutReceiver();
	return spoutReceiver->CreateReceiver(m_receiverName, m_iWidth, m_iHeight);
}

cv::Mat Opencv2Spout::receiveTexture()
{
	Mat img;
	img.create(m_iHeight, m_iWidth, CV_8UC3);
	if (spoutReceiver->ReceiveImage(m_receiverName, m_iWidth, m_iHeight, img.data, GL_BGR))
		return img;
	img.create(0, 0, CV_8UC3);
	return img;


	/*
	if (!spoutReceiver->ReceiveTexture(m_receiverName, m_iWidth, m_iHeight, 0, 0, true))
		return img;
	if(spoutReceiver->BindSharedTexture())
	{
		img.create(m_iHeight, m_iWidth, CV_8UC3);
		//use fast 4-byte alignment (default anyway) if possible
		glPixelStorei(GL_PACK_ALIGNMENT, (img.step & 3) ? 1 : 4);

		//set length of one complete row in destination data (doesn't need to equal img.cols)
		glPixelStorei(GL_PACK_ROW_LENGTH, img.step / img.elemSize());

		glReadPixels(0, 0, img.cols, img.rows, GL_BGR, GL_UNSIGNED_BYTE, img.data);

		spoutReceiver->UnBindSharedTexture();
	}
	*/
	return img;
}
Opencv2Spout::~Opencv2Spout()
{
	spout->ReleaseSender();
	if (spoutReceiver)
		spoutReceiver->ReleaseReceiver();
}
void Opencv2Spout::draw(cv::Mat &camFrame,bool drawImage)
{
	// Clear the screen and depth buffer, and reset the ModelView matrix to identity
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glLoadIdentity();

	//glEnable(GL_TEXTURE_2D);

	// Convert image and depth data to OpenGL textures
	if (drawImage)
	{
		namedWindow("OpencvSpout");
		imshow("OpencvSpout", camFrame);
	}
	else
		destroyWindow("OpencvSpout");
	Mat temp;
	cv::flip(camFrame, temp, -1);
	GLuint imageTex = matToTexture(temp, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP);

	// Draw the textures
	// Note: Window co-ordinates origin is top left, texture co-ordinate origin is bottom left.
	/*
	// Front facing texture
	glBindTexture(GL_TEXTURE_2D, imageTex);
	glBegin(GL_QUADS);
	glTexCoord2f(1, 1);
	glVertex2f(-1.0f, 1.0f);
	glTexCoord2f(0, 1);
	glVertex2f(1.0f, 1.0f);
	glTexCoord2f(0, 0);
	glVertex2f(1.0f, -1.0f);
	glTexCoord2f(1, 0);
	glVertex2f(-1.0f, -1.0f);
	glEnd();

	// Free the texture memory

	glDisable(GL_TEXTURE_2D);
	glutSwapBuffers();
	*/
	spout->SendTexture(imageTex, GL_TEXTURE_2D, 640, 480);
	glDeleteTextures(1, &imageTex);
}
GLuint Opencv2Spout::matToTexture(cv::Mat &image, GLenum minFilter, GLenum magFilter, GLenum wrapFilter)
{
	// Generate a number for our textureID's unique handle
	
	GLuint textureID;
	glGenTextures(1, &textureID);

	// Bind to our texture handle
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Catch silly-mistake texture interpolation method for magnification
	if (magFilter == GL_LINEAR_MIPMAP_LINEAR ||
		magFilter == GL_LINEAR_MIPMAP_NEAREST ||
		magFilter == GL_NEAREST_MIPMAP_NEAREST)
	{
		cout << "You can't use MIPMAPs for magnification - setting filter to GL_LINEAR" << endl;
		magFilter = GL_LINEAR;
	}

	// Set texture interpolation methods for minification and magnification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	// Set texture clamping method
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapFilter);

	// Set incoming texture format to:
	// GL_BGR       for CV_CAP_OPENNI_BGR_IMAGE,
	// GL_LUMINANCE for CV_CAP_OPENNI_DISPARITY_MAP,
	// Work out other mappings as required ( there's a list in comments in main() )
	GLenum inputColourFormat = GL_BGR;
	if (image.channels() == 1)
	{
		inputColourFormat = GL_LUMINANCE;
	}

	// Create the texture
	glTexImage2D(GL_TEXTURE_2D,     // Type of texture
		0,                 // Pyramid level (for mip-mapping) - 0 is the top level
		GL_RGB,            // Internal colour format to convert to
		image.cols,          // Image width  i.e. 640 for Kinect in standard mode
		image.rows,          // Image height i.e. 480 for Kinect in standard mode
		0,                 // Border width in pixels (can either be 1 or 0)
		inputColourFormat, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
		GL_UNSIGNED_BYTE,  // Image data type
		image.ptr());        // The actual image data itself

	// If we're using mipmaps then generate them. Note: This requires OpenGL 3.0 or higher
	if (minFilter == GL_LINEAR_MIPMAP_LINEAR ||
		minFilter == GL_LINEAR_MIPMAP_NEAREST ||
		minFilter == GL_NEAREST_MIPMAP_LINEAR ||
		minFilter == GL_NEAREST_MIPMAP_NEAREST)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	return textureID;

}
