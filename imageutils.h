#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

#include <QString>
#include <QImage>
#include <opencv2/opencv.hpp>

/*!
 * \brief The ImageUtils namespace provides image loading, color-profile handling,
 *        and simple filters using both ImageMagick and OpenCV.
 *
 *        We've also added minimal UI-styling methods for demonstration,
 *        though in a real app you typically separate styling from image utilities.
 */
namespace ImageUtils {

	/*!
	 * \brief loadAndApplyColorProfile loads an image and applies any embedded color profile.
	 * \param filePath Path to the image file.
	 * \return cv::Mat in BGR format (internally), or empty if it fails.
	 */
	cv::Mat loadAndApplyColorProfile(const QString& filePath);

	/*!
	 * \brief lanczosResizeIfNeeded performs a Lanczos resize if the image is smaller than 2000x2000,
	 *        doubling its size, otherwise keeps the same size (still applies Lanczos4).
	 * \param input The source cv::Mat.
	 * \return The resized (and BGR->RGB converted) cv::Mat.
	 */
	cv::Mat lanczosResizeIfNeeded(const cv::Mat& input);

	/*!
	 * \brief convertMatToQImage converts an RGB Mat to a QImage::Format_RGB888 QImage.
	 * \param mat A cv::Mat in RGB format.
	 * \return A QImage in RGB888.
	 */
	QImage convertMatToQImage(const cv::Mat& mat);

	/*!
	 * \brief convertQImageToMat converts a QImage to a cv::Mat in BGR format.
	 * \param image A QImage (any format).
	 * \return A cv::Mat in BGR format.
	 */
	cv::Mat convertQImageToMat(const QImage& image);

	/*!
	 * \brief convertToGrayscale converts a QImage to grayscale using the luminosity method.
	 * \param image Source QImage (RGB or ARGB).
	 * \return A grayscale QImage with alpha preserved.
	 */
	QImage convertToGrayscale(const QImage& image);

	/*!
	 * \brief posterize lowers the color resolution of an image to a specified number of levels.
	 * \param image The source QImage.
	 * \param levels The number of discrete levels (e.g. 8).
	 * \return A posterized QImage.
	 */
	QImage posterize(const QImage& image, int levels, bool normalizeAB = false);

	/*!
	 * \brief gaussianBlur applies a Gaussian blur using OpenCV.
	 * \param image The source QImage.
	 * \param kernelSize The sigma for the blur (OpenCV uses dynamic kernel).
	 * \return The blurred QImage.
	 */
	QImage gaussianBlur(const QImage& image, int kernelSize);

	/*!
	 * \brief medianFilter applies a median filter with kernel size depending on image size.
	 * \param image The source QImage.
	 * \return A median-filtered QImage.
	 */
	QImage medianFilter(const QImage& image);

	/*!
	 * \brief applyGlobalStyleSheet applies a global stylesheet (QSS) to the QApplication.
	 *        Demonstrates loading from a resource or inline string.
	 *        In a real app, this might live in main.cpp or a separate "Theme" module.
	 * \param qssFilePath Path to a .qss file resource, or empty to apply a default example.
	 */
	void applyGlobalStyleSheet(const QString& qssFilePath = QString());

} // namespace ImageUtils

#endif // IMAGEUTILS_H
