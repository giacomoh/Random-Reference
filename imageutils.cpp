#include "imageutils.h"

#include <QFile>
#include <QDebug>
#include <QtGlobal>
#include <QColor>
#include <QByteArray>
#include <QImage>

#include <MagickWand.h>
#include <lcms2.h>
#include <opencv2/opencv.hpp>

// For applying style to the QApplication
#include <QApplication>
#include <QFile>
#include <QDir>

namespace ImageUtils {

    cv::Mat loadAndApplyColorProfile(const QString& filePath)
    {
        // Prepare Magick
        MagickWandGenesis();
        MagickWand* wand = NewMagickWand();

        // Load file
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Cannot open file:" << filePath;
            return cv::Mat();
        }
        QByteArray data = file.readAll();
        file.close();

        if (MagickReadImageBlob(wand, data.constData(), data.size()) == MagickFalse) {
            qWarning() << "Failed to read image via Magick.";
            DestroyMagickWand(wand);
            MagickWandTerminus();
            return cv::Mat();
        }

        // Attempt to retrieve the color profile
        size_t length;
        unsigned char* profile = MagickGetImageProfile(wand, "ICC", &length);
        cv::Mat img;

        if (profile) {
            qDebug() << "Color profile found.";
            cmsHPROFILE inProfile = cmsOpenProfileFromMem(profile, length);
            cmsHPROFILE outProfile = cmsCreate_sRGBProfile();
            cmsHTRANSFORM transform = cmsCreateTransform(inProfile, TYPE_BGR_8, outProfile, TYPE_BGR_8, INTENT_PERCEPTUAL, 0);

            // read with OpenCV
            img = cv::imread(filePath.toStdString(), cv::IMREAD_COLOR);
            if (!img.empty()) {
                // Apply transform in place
                cmsDoTransform(transform, img.data, img.data, img.total());
            }
            cmsDeleteTransform(transform);
            cmsCloseProfile(inProfile);
            cmsCloseProfile(outProfile);
        }
        else {
            qDebug() << "No color profile found.";
            img = cv::imread(filePath.toStdString(), cv::IMREAD_COLOR);
        }

        DestroyMagickWand(wand);
        MagickWandTerminus();
        return img;
    }

    cv::Mat lanczosResizeIfNeeded(const cv::Mat& input)
    {
        if (input.empty()) return cv::Mat();

        cv::Mat output;
        if (input.cols < 2000 && input.rows < 2000) {
            // If smaller than 2000x2000, double the size with Lanczos
            cv::resize(input, output, cv::Size(input.cols * 2, input.rows * 2), 0, 0, cv::INTER_LANCZOS4);
        }
        else {
            // Otherwise, keep same size but still use Lanczos for consistency
            cv::resize(input, output, cv::Size(input.cols, input.rows), 0, 0, cv::INTER_LANCZOS4);
        }
        // Convert BGR -> RGB for QImage
        cv::cvtColor(output, output, cv::COLOR_BGR2RGB);
        return output;
    }

    QImage convertMatToQImage(const cv::Mat& mat)
    {
        if (mat.empty()) return QImage();

        // Here we assume mat is in RGB
        return QImage(mat.data,
            mat.cols,
            mat.rows,
            static_cast<int>(mat.step),
            QImage::Format_RGB888).copy();
    }

    cv::Mat convertQImageToMat(const QImage& image)
    {
        QImage converted = image.convertToFormat(QImage::Format_RGB888);
        cv::Mat mat(converted.height(), converted.width(), CV_8UC3,
            (uchar*)converted.bits(), converted.bytesPerLine());
        cv::Mat matCopy;
        // Convert RGB -> BGR
        cv::cvtColor(mat, matCopy, cv::COLOR_RGB2BGR);
        return matCopy;
    }

    QImage convertToGrayscale(const QImage& image)
    {
        QImage gray(image.size(), QImage::Format_ARGB32);
        for (int y = 0; y < gray.height(); ++y) {
            for (int x = 0; x < gray.width(); ++x) {
                QColor c = image.pixelColor(x, y);
                int g = qRound(0.299 * c.red() + 0.587 * c.green() + 0.114 * c.blue());
                gray.setPixelColor(x, y, QColor(g, g, g, c.alpha()));
            }
        }
        return gray;
    }

    QImage posterize(const QImage& image, int levels, bool normalizeAB)
    {
        if (levels < 2) {
            qDebug() << "Posterize levels must be at least 2. Returning original image.";
            return image;
        }

        // Convert QImage to cv::Mat
        cv::Mat mat = convertQImageToMat(image);
        if (mat.empty()) {
            qWarning() << "Empty image provided for posterization.";
            return image;
        }

        // Convert BGR to CIELAB
        cv::Mat lab;
        cv::cvtColor(mat, lab, cv::COLOR_BGR2Lab);

        // Split into channels: channels[0]=L*, channels[1]=a*, channels[2]=b*
        std::vector<cv::Mat> channels;
        cv::split(lab, channels);

        // Convert L* to float for precision
        cv::Mat L_float;
        channels[0].convertTo(L_float, CV_32F);

        // Define quantization interval based on OpenCV’s Lab scaling (0-255)
        double interval = 255.0 / levels;

        // Build a lookup table for the quantized L* value:
        std::vector<float> lut(levels, 0.0f);
        for (int i = 0; i < levels; ++i) {
            lut[i] = (i + 0.5f) * interval;
            if (lut[i] > 255.0f)
                lut[i] = 255.0f;
        }

        // Quantize L* channel using the lookup table
        cv::Mat posterized_L = L_float.clone();
        for (int i = 0; i < posterized_L.rows; ++i) {
            float* ptr = posterized_L.ptr<float>(i);
            for (int j = 0; j < posterized_L.cols; ++j) {
                float original_L = ptr[j];
                int bin = static_cast<int>(std::floor(original_L / interval));
                if (bin >= levels)
                    bin = levels - 1;
                ptr[j] = lut[bin];
            }
        }

        // Convert the posterized L* channel back to 8-bit
        cv::Mat posterized_L_uchar;
        posterized_L.convertTo(posterized_L_uchar, CV_8U);

        // Replace the original L* channel
        channels[0] = posterized_L_uchar;

        // Optional: Normalize the a* and b* channels within each bin
        if (normalizeAB) {
            std::vector<cv::Vec2f> avg_ab(levels, cv::Vec2f(0.0f, 0.0f));
            std::vector<int> count(levels, 0);

            for (int i = 0; i < posterized_L_uchar.rows; ++i) {
                for (int j = 0; j < posterized_L_uchar.cols; ++j) {
                    uchar l = posterized_L_uchar.at<uchar>(i, j);
                    int bin = static_cast<int>(std::floor(static_cast<float>(l) / interval));
                    if (bin >= levels)
                        bin = levels - 1;
                    avg_ab[bin][0] += channels[1].at<uchar>(i, j);
                    avg_ab[bin][1] += channels[2].at<uchar>(i, j);
                    count[bin]++;
                }
            }
            for (int i = 0; i < levels; ++i) {
                if (count[i] > 0) {
                    avg_ab[i][0] /= static_cast<float>(count[i]);
                    avg_ab[i][1] /= static_cast<float>(count[i]);
                }
            }
            for (int i = 0; i < posterized_L_uchar.rows; ++i) {
                for (int j = 0; j < posterized_L_uchar.cols; ++j) {
                    uchar l = posterized_L_uchar.at<uchar>(i, j);
                    int bin = static_cast<int>(std::floor(static_cast<float>(l) / interval));
                    if (bin >= levels)
                        bin = levels - 1;
                    channels[1].at<uchar>(i, j) = static_cast<uchar>(avg_ab[bin][0]);
                    channels[2].at<uchar>(i, j) = static_cast<uchar>(avg_ab[bin][1]);
                }
            }
        }

        // Merge channels back into a Lab image
        cv::Mat labPosterized;
        cv::merge(channels, labPosterized);

        // Convert Lab back to BGR
        cv::Mat posterizedBGR;
        cv::cvtColor(labPosterized, posterizedBGR, cv::COLOR_Lab2BGR);

        // Convert BGR to RGB for QImage
        cv::Mat posterizedRGB;
        cv::cvtColor(posterizedBGR, posterizedRGB, cv::COLOR_BGR2RGB);

        // ***** Noise removal step *****
        // To eliminate isolated "lone" pixels, apply a small morphological opening.
        int morphKernelSize = 3; // Choose 3x3; adjust as needed
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(morphKernelSize, morphKernelSize));
        cv::Mat cleaned;
        cv::morphologyEx(posterizedRGB, cleaned, cv::MORPH_OPEN, kernel);

        // Convert the cleaned image to QImage for output
        QImage posterizedImage = convertMatToQImage(cleaned);
        return posterizedImage;
    }

    QImage gaussianBlur(const QImage& image, int kernelSize)
    {
        if (image.isNull()) return QImage();

        cv::Mat mat = convertQImageToMat(image);
        cv::GaussianBlur(mat, mat, cv::Size(0, 0), kernelSize);
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return convertMatToQImage(rgb);
    }

    QImage medianFilter(const QImage& image)
    {
        if (image.isNull()) return QImage();

        cv::Mat mat = convertQImageToMat(image);
        int kSize = 1;
        // Choose a kernel size based on image resolution
        if (mat.cols < 1000 && mat.rows < 1000)
            kSize = 1;
        else if (mat.cols < 2000 && mat.rows < 2000)
            kSize = 3;
        else
            kSize = 5;
        cv::medianBlur(mat, mat, kSize);
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return convertMatToQImage(rgb);
    }

    void applyGlobalStyleSheet(const QString& qssFilePath)
    {
        if (QApplication::instance() == nullptr) {
            qWarning() << "No QApplication found. Cannot apply stylesheet.";
            return;
        }
        QApplication* app = qobject_cast<QApplication*>(QApplication::instance());
        if (!app) {
            qWarning() << "The application instance is not a QApplication.";
            return;
        }
        if (!qssFilePath.isEmpty()) {
            QFile file(qssFilePath);
            if (file.open(QFile::ReadOnly)) {
                QString styleContent = QString::fromUtf8(file.readAll());
                app->setStyleSheet(styleContent);
                qDebug() << "Applied stylesheet from file:" << qssFilePath;
                return;
            }
            else {
                qWarning() << "Failed to open QSS file:" << qssFilePath;
            }
        }
        QString defaultStyle = R"(
QMainWindow {
    background-color: #2b2b2b;
}
QPushButton {
    background-color: #3c8dbc;
    color: white;
    border: 1px solid #337ab7;
    padding: 6px;
    border-radius: 4px;
    font: bold 14px "Arial";
}
QPushButton:hover {
    background-color: #2491d1;
}
QLabel {
    color: #ffffff;
}
)";
        app->setStyleSheet(defaultStyle);
        qDebug() << "Applied default inline stylesheet.";
    }

} // namespace ImageUtils
