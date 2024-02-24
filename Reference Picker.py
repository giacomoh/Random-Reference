import sys
print(sys.executable)
import os
import random
from PyQt5 import QtWidgets, QtCore, QtGui
import pyqtgraph as pg
import numpy as np
from PIL import Image
from scipy.ndimage import median_filter
import glob

class ImageViewer(QtWidgets.QMainWindow):
    
    def __init__(self, parent=None):
        super(ImageViewer, self).__init__(parent)
        self.setWindowFlags(self.windowFlags() | QtCore.Qt.WindowStaysOnTopHint)  # Add this line
        self.initialize_variables()
        self.setup_view()
        self.load_settings()
        self.update_images()
        self.setup_toolbar()
        self.timer_running = True
        self.posterize_levels = 3

    def initialize_variables(self):
        self.img = None
        self.image_history = []
        self.current_image_index = None
        self.images = []
        self.shuffled_images = []
        self.is_mirrored = False
        self.transformations = []
        self.folder_path = ""
        # Initialize the transformation status dictionary
        self.transformation_status = {
            self.greyscale_transform: False,
            self.median_filter_transform: False,
            self.posterize_transform: False,
            self.mirrored: False 
        }

    def setup_view(self):
        self.view = pg.ImageView()
        vb = self.view.getView()
        darker_grey = QtGui.QColor(70, 70, 70)
        darker_grey_str = darker_grey.name()
        vb.setBackgroundColor(darker_grey)
        self.view.ui.roiBtn.hide()
        self.view.ui.menuBtn.hide()
        self.view.ui.histogram.hide()
        self.setCentralWidget(self.view)

    def load_settings(self):
        self.settings = QtCore.QSettings("Hwllw", "RandomRefPicker")
        self.restoreGeometry(self.settings.value("geometry", QtCore.QByteArray()))
        self.restoreState(self.settings.value("windowState", QtCore.QByteArray()))
        lastFolderPath = self.settings.value("lastFolderPath", "")
        if lastFolderPath:  # Check if lastFolderPath is not an empty string
            self.folder_path = lastFolderPath
            print(f"Last saved folder path: {self.folder_path}")
            self.new_image()

    def add_button(self, layout, text, callback, icon_path, style):
        button = QtWidgets.QPushButton(text)
        button.clicked.connect(callback)
        button.setIcon(QtGui.QIcon(icon_path))
        button.setSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding)

        # Add CSS styles
        button.setStyleSheet("""
            QPushButton {
                color: black;
                background-color: #5D5D5D;
                border: 2px solid #2E2E2E;
                border-radius: 10px;
                padding: 5px;
                font-size: 20px;
                font-family: Arial;
            }
            QPushButton:hover {
                background-color: #7F7F7F;
            }
            QPushButton:pressed {
                background-color: #3F3F3F;
            }
        """)

        layout.addWidget(button)
        return button

    def setup_toolbar(self):
        darker_grey = QtGui.QColor(70, 70, 70)
        darker_grey_str = darker_grey.name()


        style = f"background-color: {darker_grey_str}; color: black; font-size: 20px;"
    
        # Create the first toolbar
        toolbar1 = self.addToolBar("Toolbar1")
        toolbar1.setObjectName("UI1")
        toolbar1.setMovable(False)  # Make toolbar1 non-movable
        widget1 = QtWidgets.QWidget()
        layout1 = QtWidgets.QHBoxLayout()
        toolbar1.setStyleSheet(f"background-color: {darker_grey_str}; border: 1px solid #000000")
        widget1.setLayout(layout1)

        # Add buttons to the first toolbar
        self.add_button(layout1, "📂", self.button1_clicked, 'path_to_icon', style)
        self.add_button(layout1, "⏩", self.button2_clicked, 'path_to_icon', style)
        self.add_button(layout1, "⏪", self.button3_clicked, 'path_to_icon', style)
        self.timer_button = self.add_button(layout1, "00:00:00", self.button10_clicked, 'path_to_icon', style)
        self.timer_button.setMinimumSize(100, 30)  # Set the minimum size of self.timer_button
        self.add_button(layout1, "⏱️", self.convert_to_time, 'path_to_icon', style)


        
        # Add self.hours_input to the first toolbar
        self.hours_input = QtWidgets.QLineEdit()
        self.hours_input.setStyleSheet("font-size: 30px; color: black")  # Set font size and color
        self.hours_input.setMaximumWidth(50)  # Set maximum width
        self.hours_input.setMinimumHeight(30)  # Set minimum height
        layout1.addWidget(self.hours_input)  # Add self.hours_input to the first toolbar

        # Add self.minutes_input to the first toolbar
        self.minutes_input = QtWidgets.QLineEdit()
        self.minutes_input.setStyleSheet("font-size: 30px; color: black")  # Set font size and color
        self.minutes_input.setMaximumWidth(50)  # Set maximum width
        self.minutes_input.setMinimumHeight(30)  # Set minimum height
        layout1.addWidget(self.minutes_input)

        # Add self.seconds_input to the first toolbar
        self.seconds_input = QtWidgets.QLineEdit()
        self.seconds_input.setStyleSheet("font-size: 30px; color: black")  # Set font size and color
        self.seconds_input.setMaximumWidth(50)  # Set maximum width
        self.seconds_input.setMinimumHeight(30)  # Set minimum height
        layout1.addWidget(self.seconds_input)

        self.timer = QtCore.QTimer(self)
        self.timer.timeout.connect(self.update_timer)
        self.time = QtCore.QTime(0, 0)
        self.timer.stop()

        toolbar1.addWidget(widget1)
        
        self.hide_ui_button = self.add_button(layout1, "🔼", self.toggle_toolbar2, 'path_to_icon', style)
        
        # Add a break
        self.addToolBarBreak()

        # Create the second toolbar
        self.toolbar2 = self.addToolBar("Toolbar2")
        self.toolbar2.setObjectName("UI2")
        self.toolbar2.setMovable(False)  # Make toolbar2 non-movable
        widget2 = QtWidgets.QWidget()
        layout2 = QtWidgets.QHBoxLayout()
        self.toolbar2.setStyleSheet(f"background-color: {darker_grey_str}; border: 1px solid #000000")
        widget2.setLayout(layout2)

        # Add buttons to the second toolbar
        self.add_button(layout2, "greyscale", self.button4_clicked, 'path_to_icon', style)
        self.add_button(layout2, "flip", self.button5_clicked, 'path_to_icon', style)
        self.add_button(layout2, "rotate", self.button6_clicked, 'path_to_icon', style)
        self.add_button(layout2, "reset", self.button7_clicked, 'path_to_icon', style)
        self.add_button(layout2, "posterize", self.button8_clicked, 'path_to_icon', style)

        # Add a QSpinBox for the posterize levels
        self.posterize_spinbox = QtWidgets.QSpinBox()
        self.posterize_spinbox.setMinimum(1)
        self.posterize_spinbox.setMaximum(10)
        self.posterize_spinbox.setValue(3)  # Default value

        # Add a QSpinBox for the posterize levels
        self.posterize_spinbox = QtWidgets.QSpinBox()
        self.posterize_spinbox.setMinimum(1)
        self.posterize_spinbox.setMaximum(10)
        self.posterize_spinbox.setValue(3)  # Default value

        # Set the size policy
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setVerticalStretch(2)  # Adjust the vertical stretch factor
        self.posterize_spinbox.setSizePolicy(sizePolicy)

        # Connect the valueChanged signal to a new method
        self.posterize_spinbox.valueChanged.connect(self.update_posterize_levels)

        # Set the text color to white
        self.posterize_spinbox.setStyleSheet("font-size: 20px; color: white")  # Set font size and color

        layout2.addWidget(self.posterize_spinbox)

        self.toolbar2.addWidget(widget2)

    def toggle_toolbar2(self):
        # Toggle the visibility of toolbar2
        self.toolbar2.setVisible(not self.toolbar2.isVisible())

    def update_timer(self):
        if not self.time.toString() == "00:00:00":
            self.time = self.time.addSecs(-1)
        else:
            self.timer.stop()
            self.new_image()  # Call pick_random_image without passing a directory path
            self.convert_to_time()

        self.timer_button.setText(self.time.toString("hh:mm:ss"))

    def closeEvent(self, event):
        # Save window size and position
        self.settings.setValue("geometry", self.saveGeometry())
        self.settings.setValue("windowState", self.saveState())
        super(ImageViewer, self).closeEvent(event)

    def create_button(self, text, handler, icon_path, style):
        button = QtWidgets.QToolButton()
        button.setText(text)
        button.clicked.connect(handler)
        button.setIcon(QtGui.QIcon(icon_path))
        button.setStyleSheet(style)
        button.setToolButtonStyle(QtCore.Qt.ToolButtonTextUnderIcon)
        return button
    
    def convert_to_time(self):
        hours = int(self.hours_input.text()) if self.hours_input.text() else 0
        minutes = int(self.minutes_input.text()) if self.minutes_input.text() else 0
        seconds = int(self.seconds_input.text()) if self.seconds_input.text() else 0
        total_seconds = hours * 3600 + minutes * 60 + seconds
        if total_seconds > 10:
            self.time.setHMS(hours, minutes, seconds) # Stop the timer
            self.timer.start(1000)  # Start the timer againù
    
    def update_images(self):
        if self.folder_path:  # Check if folder_path is not an empty string
            self.images = glob.glob(self.folder_path + '/**/*', recursive=True)
            self.images = [img for img in self.images if img.endswith(('.png', '.jpg', '.jpeg'))]

    def load_image(self, img_path):
        # Load the image using PIL
        pil_img = Image.open(img_path)
        

        # Apply Lanczos resampling
        factor = 2  # Define the factor for Lanczos resampling
        new_size = (int(pil_img.size[0] * factor), int(pil_img.size[1] * factor))
        pil_img = pil_img.resize(new_size, Image.LANCZOS)

        # Convert the PIL Image to a numpy array
        img = np.array(pil_img)

        self.img = np.rot90(img, 3)
        self.img = np.fliplr(self.img)

        # Store the original image before any transformations are applied
        self.original_img = self.img.copy()

        # Clear the transformations list and reset the transformation status dictionary
        self.transformations.clear()
        self.transformation_status = {
            'greyscale_transform': False,
            'median_filter_transform': False,
            'posterize_transform': False,
            'mirrored': False
        }
        # Display the image
        self.view.setImage(self.img)

    def apply_transformations(self):
        self.img = self.original_img.copy()
        for transform in self.transformations:
            self.img = transform(self.img)
        self.view.setImage(self.img)
    
    def greyscale_transform(self, img):
        if img.shape[-1] in [3, 4]:
            return np.dot(img[...,:3], [0.2989, 0.5870, 0.1140])[..., np.newaxis]
        else:
            return img

    def update_posterize_levels(self, value):
        # Update the posterize levels
        self.posterize_levels = value
        view_state = self.view.getView().getState()

        # Apply the posterize transformation to the original image
        transformed_img = self.posterize_transform(self.original_img.copy())
        # Update the image in the view
        self.view.setImage(transformed_img)

        self.view.getView().setState(view_state)

        # Set the posterize_transform flag to True
        self.transformation_status['posterize_transform'] = True

    def posterize_transform(self, img):
        if self.posterize_levels < 1:
            raise ValueError("Levels must be greater than 0")

        if img.ndim < 2:
            raise ValueError("Image must be at least 2D")

        # Calculate the factor to reduce the color depth
        factor = 256.0 / self.posterize_levels

        # Quantize each color channel
        img = (img / factor).astype(int) * factor

        return img
    

    def median_filter_transform(self, img, size=3):
        # Split the image into color channels
        channels = np.dsplit(img, img.shape[-1])

        # Apply the median filter to each channel
        filtered_channels = [median_filter(channel, size) for channel in channels]

        # Recombine the channels
        img = np.dstack(filtered_channels)

        return img
    def shift_image_to_center(self, img):
        shift_x = img.shape[1] // 2
        shift_y = img.shape[0] // 2
        return np.roll(np.roll(img, shift_y, axis=0), shift_x, axis=1)
    
    def new_image(self):
        # Update the list of images before selecting a new one
        self.update_images()

        if self.images:
            # If we've gone through all the images, shuffle them again
            if not self.shuffled_images:
                self.shuffled_images = random.sample(self.images, len(self.images))

            # Select the next image from the shuffled list
            chosen_image = self.shuffled_images.pop()
            print(f"Image selected: {chosen_image}")

            # Add the chosen image to the history and update the current image index
            self.image_history.append(chosen_image)
            self.current_image_index = len(self.image_history) - 1

            self.load_image(chosen_image)
        else:
            print("No images found in the selected folder.")
            
    def button1_clicked(self):
        self.folder_path = QtWidgets.QFileDialog.getExistingDirectory(self, 'Select Folder')
        print(f"Folder selected: {self.folder_path}")

        # Save the last folder path
        self.settings.setValue("lastFolderPath", self.folder_path)
        status = self.settings.status()
        if status == QtCore.QSettings.AccessError:
            print("Error: Could not write settings due to an access error.")
        elif status == QtCore.QSettings.FormatError:
            print("Error: Could not write settings due to a format error.")
        else:
            print("Settings were written successfully.")
        
        # Update the list of images
        self.update_images()

        # Clear the shuffled images list so that it gets refreshed the next time button2 is clicked
        self.shuffled_images.clear()

        # Load a new image
        self.new_image()

    def button2_clicked(self):
        self.new_image()    

    def button3_clicked(self):
        if self.current_image_index is not None and self.current_image_index > 0:
            # Go back to the previous image
            self.current_image_index -= 1
            previous_image = self.image_history[self.current_image_index]
            print(f"Image selected: {previous_image}")
            self.load_image(previous_image)
        else:
            print("No previous image.")
    

    def button4_clicked(self):
        if self.img is not None:
            # Save the current view state
            view_state = self.view.getView().getState()

            if self.transformation_status["greyscale_transform"]:
                # If the image is currently in greyscale, revert it to the original
                if self.greyscale_transform in self.transformations:
                    self.transformations.remove(self.greyscale_transform)
                self.transformation_status["greyscale_transform"] = False
            else:
                # If the image is not in greyscale, convert it to greyscale and store the original
                self.transformations.append(self.greyscale_transform)
                self.transformation_status["greyscale_transform"] = True

            self.apply_transformations()

            # Restore the view state
            self.view.getView().setState(view_state)
            current_center_x = (view_state['targetRange'][0][0] + view_state['targetRange'][0][1]) / 2
            print(f"Current center: {current_center_x}")
        else:
            print("No image loaded.")

    def mirrored(self, img):
        if self.view is not None:
            # Get the ImageItem from the ImageView
            image_item = self.view.getImageItem()

            # Get the current transformation
            transform = image_item.transform()

            # Get the ViewBox from the ImageView
            view_box = self.view.getView()

            # Get the center of the view
            center = view_box.viewRect().center()

            # Create a mirror transformation
            mirror = QtGui.QTransform(-1, 0, 0, 1, 2*center.x(), 0)

            # If the image is currently mirrored, revert it to the original
            if self.is_mirrored:
                image_item.setTransform(transform * mirror.inverted()[0])
                self.is_mirrored = False
                return img
            else:
                # If the image is not mirrored, apply the mirror transformation
                image_item.setTransform(transform * mirror)
                self.is_mirrored = True
                return np.flipud(img)
        else:
            return img

    def button5_clicked(self):
        self.img = self.mirrored(self.img)
            
    def button6_clicked(self):
        if self.img is not None:
            view_state = self.view.getView().getState()
            self.transformations.append(lambda img: np.rot90(img, 1, axes=(1, 0)))
            self.apply_transformations()
            self.view.getView().setState(view_state)

    def button7_clicked(self):
        if self.img is not None:
            # Clear the transformations list
            self.transformations.clear()

            # Reset the image to the original
            self.img = self.original_img.copy()

            # Reset the greyscale and posterize flags
            self.is_greyscale = False
            self.is_posterized = False

            # Update the image view
            self.view.setImage(self.img)
        else:
            print("No image loaded.")

    def button8_clicked(self):
        if self.img is not None:
            # Save the current view state
            view_state = self.view.getView().getState()

            if self.transformation_status["posterize_transform"]:
                # If the image is currently posterized, revert it to the original
                if self.posterize_transform in self.transformations:
                    self.transformations.remove(self.posterize_transform)
                self.transformation_status["posterize_transform"] = False
            else:
                # If the image is not posterized, convert it to posterized and store the original
                self.transformations.append(self.posterize_transform)
                self.transformation_status["posterize_transform"] = True

            self.apply_transformations()

            # Restore the view state
            self.view.getView().setState(view_state)
        else:
            print("No image loaded.")
    
    def button9_clicked(self):
        if self.img is not None:
            # Save the current view state
            view_state = self.view.getView().getState()

            if self.transformation_status['median_filter_transform']:
                # If the image is currently filtered, revert it to the original
                if self.median_filter_transform in self.transformations:
                    self.transformations.remove(self.median_filter_transform)
                self.transformation_status['median_filter_transform'] = False
            else:
                # If the image is not filtered, apply the median filter and store the original
                self.transformations.append(self.median_filter_transform)
                self.transformation_status['median_filter_transform'] = True

            self.apply_transformations()

            # Restore the view state
            self.view.getView().setState(view_state)
        else:
            print("No image loaded.")    

    def button10_clicked(self):
        if self.timer_running:
            self.timer.stop()
            print("timer stop")
            self.timer_button.setStyleSheet("""
            QPushButton {
                color: red;
                background-color: #5D5D5D;
                border: 2px solid #2E2E2E;
                border-radius: 10px;
                padding: 5px;
                font-size: 20px;
                font-family: Arial;
            }
            QPushButton:hover {
                background-color: #7F7F7F;
            }
            QPushButton:pressed {
                background-color: #3F3F3F;
            }
        """)
            
        else:
            self.timer.start(1000)
            print("timer start")
            self.timer_button.setStyleSheet("""
            QPushButton {
                color: black;
                background-color: #5D5D5D;
                border: 2px solid #2E2E2E;
                border-radius: 10px;
                padding: 5px;
                font-size: 20px;
                font-family: Arial;
            }
            QPushButton:hover {
                background-color: #7F7F7F;
            }
            QPushButton:pressed {
                background-color: #3F3F3F;
            }
        """)
        self.timer_running = not self.timer_running  # Toggle the value




def main():
    app = QtWidgets.QApplication(sys.argv)

    mainWin = ImageViewer()
    mainWin.show()

    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
