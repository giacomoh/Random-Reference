import os
import random
from tkinter import filedialog
from PIL import Image, ImageTk, ImageOps
import customtkinter
import time
import threading
from scipy.ndimage import median_filter
import numpy as np

class ImageViewer:
    def __init__(self):
        self.selected_folder = self.load_folder_path()  # Load the last selected folder path
        self.current_image = None
        self.current_photo = None
        self.canvas = None
        self.prev_x = None
        self.prev_y = None
        self.scale_factor = 1.0
        self.pan_x = 0
        self.pan_y = 0
        self.flipped_horizontal = False
        self.bind_mouse_events()
        self.original_image = None
        self.is_grayscale = False
        self.image_history = []  # History of selected images
        self.history_position = -1  # Current position in the history
        self.original_rotation = 0  
        self.timer = None  # Initialize self.timer
        self.timer_delay = 1000  # Delay before picking a new image (in milliseconds)
        self.timer_id = None  # ID of the scheduled function call-
        self.m_time_entry = 0
        self.s_time_entry = 0
        self.median_filter_applied = False
        self.original_image = None
        self.original_image_grayscale = None
        self.original_image_median = None
        self.original_image_posterize = None
        self.is_posterized = False  # Add this line
        self.median_filter_button = None  # Initialize the median filter button
        self.posterize_button = None  # Initialize the posterize button
        self.grayscale_button = None  # Initialize the grayscale button

    
        if self.selected_folder:
            self.pick_image()  # Automatically pick an image if a folder is already selected
            self.bind_mouse_events()
            if self.canvas:
                self.canvas.bind("<MouseWheel>", self.on_mouse_wheel)

    def load_folder_path(self):
        if os.path.exists('last_opened_folder.txt'):
            with open('last_opened_folder.txt', 'r') as f:
                return f.read().strip()  # Remove any leading/trailing whitespace
        else:
            return None  # or return an empty string ""

    def save_folder_path(self, path):
        with open('last_opened_folder.txt', 'w') as f:
            f.write(path)

    def reset_button_colors(self):
        # Reset the color of the median filter button
        if self.median_filter_button:
            self.median_filter_button.configure(fg_color="#2D2D2D")

        # Reset the color of the posterize button
        if self.posterize_button:
            self.posterize_button.configure(fg_color="#2D2D2D")
        
        if self.grayscale_button:
            self.grayscale_button.configure(fg_color="#2D2D2D")

    def select_folder(self):
        folder_path = filedialog.askdirectory()
        if folder_path:
            self.selected_folder = folder_path
            self.save_folder_path(folder_path)  # Save the selected folder path
            print("Selected folder:", folder_path)

    def pick_image(self):
        try:
            if self.selected_folder:
                image_filename = random.choice(os.listdir(self.selected_folder))
                print("Selected image:", image_filename)
                file_path = os.path.join(self.selected_folder, image_filename)
                self.current_image = Image.open(file_path)
                self.current_photo = ImageTk.PhotoImage(self.current_image)
                self.display_image()
                self.reset_pan()
                self.reset_scale()
                self.flipped_horizontal = False
                self.bind_mouse_events()
                self.canvas.bind("<MouseWheel>", self.on_mouse_wheel)

                # Add the selected image to the history and update the history position
                self.image_history.append(file_path)
                self.history_position += 1
                print(self.history_position)

                # Reset the median filter
                self.median_filter_applied = False
                self.original_image = None

            else:
                print("Please select a folder first.")
        except Exception as e:
            print("An error occurred:", e)
        self.original_image = self.current_image
        self.is_grayscale = False

    def go_back(self):
        if self.history_position > 0:
            # Decrease the history position and load the previous image
            self.history_position -= 1
            file_path = self.image_history[self.history_position]
            self.current_image = Image.open(file_path)
            self.current_photo = ImageTk.PhotoImage(self.current_image)
            self.display_image()
            self.reset_pan()
            self.reset_scale()
            self.flipped_horizontal = False
            self.bind_mouse_events()
            self.canvas.bind("<MouseWheel>", self.on_mouse_wheel)

            # Reset the median filter
            self.median_filter_applied = False
            self.original_image = None

            print(self.history_position)

        else:
            print("No previous image.")
    def on_move_press(self, event):
        dx = event.x - self.prev_x
        dy = event.y - self.prev_y
        self.prev_x = event.x
        self.prev_y = event.y
        self.pan_x += dx
        self.pan_y += dy
        self.bound_panning()
        self.display_image()

    def display_image(self):
        if self.current_photo:
            # Apply the scale to the image
            scaled_image = self.current_image.resize((int(self.current_image.width * self.scale_factor), int(self.current_image.height * self.scale_factor)))
            self.current_photo = ImageTk.PhotoImage(scaled_image)

            # Display the image
            self.canvas.config(width=self.current_photo.width(), height=self.current_photo.height())
            self.canvas.delete("all")
            self.canvas.create_image(self.pan_x, self.pan_y, anchor="nw", image=self.current_photo)

    def rotate_image(self):
        if self.current_photo and self.current_image:
            # Rotate the image by 90 degrees clockwise
            self.current_image = self.current_image.rotate(-90, expand=True)

            # Scale the image
            scaled_image = self.current_image.resize((int(self.current_image.width * self.scale_factor), int(self.current_image.height * self.scale_factor)))
            self.current_photo = ImageTk.PhotoImage(scaled_image)

            # Get the current canvas dimensions
            canvas_width = self.canvas.winfo_width()
            canvas_height = self.canvas.winfo_height()

            self.display_image()

    def flip_horizontal(self):
        if self.current_photo and self.current_image:
            flipped_image = ImageOps.mirror(self.current_image)
            self.current_image = flipped_image
            scaled_image = flipped_image.resize((int(flipped_image.width * self.scale_factor), int(flipped_image.height * self.scale_factor)))
            self.current_photo = ImageTk.PhotoImage(scaled_image)

            # Get the current canvas dimensions
            canvas_width = self.canvas.winfo_width()
            canvas_height = self.canvas.winfo_height()

            # Calculate the new pan coordinates
            self.pan_x = canvas_width - (self.pan_x + scaled_image.width)

            self.display_image()
            self.flipped_horizontal = not self.flipped_horizontal

    def on_button_press(self, event):
        self.prev_x = event.x
        self.prev_y = event.y

    def on_mouse_wheel(self, event):
        oldscale = self.scale_factor
        self.scale_factor += event.delta / 1200.0
        if self.scale_factor < 0.1:
            self.scale_factor = oldscale
    
        if event.delta > 0:
            self.scale_factor *= 1.05
        else:
            self.scale_factor /= 1.05

        # Get the current mouse coordinates
        mouse_x = self.canvas.canvasx(event.x)
        mouse_y = self.canvas.canvasy(event.y)

        # Calculate the new image dimensions
        new_width = int(self.current_image.width * self.scale_factor)
        new_height = int(self.current_image.height * self.scale_factor)

        # Calculate the new pan coordinates
        self.pan_x = mouse_x - (mouse_x - self.pan_x) * (new_width / (self.current_image.width * oldscale))
        self.pan_y = mouse_y - (mouse_y - self.pan_y) * (new_height / (self.current_image.height * oldscale))

        # Resize the image and display it
        scaled_image = self.current_image.resize((new_width, new_height))
        self.current_photo = ImageTk.PhotoImage(scaled_image)
        self.display_image()
        
    def reset_image(self):
        if self.current_image:
             # Reset the image to the original image before any rotation
            self.current_image = self.original_image
            self.original_rotation = 0  # Reset the rotation
            self.current_image = self.original_image
            self.current_photo = ImageTk.PhotoImage(self.current_image)
            self.display_image()
            self.reset_pan()
            self.reset_scale()
            self.is_grayscale = False
            self.center_image()
            self.reset_button_colors()
    def center_image(self):
        if self.current_photo:
            canvas_width = self.canvas.winfo_width()
            canvas_height = self.canvas.winfo_height()
            image_width = self.current_photo.width()
            image_height = self.current_photo.height()

            self.pan_x = (canvas_width - image_width) / 2
            self.pan_y = (canvas_height - image_height) / 2
            self.display_image()

    def resize_canvas(self, event=None):
        if self.current_photo:
            self.canvas.config(width=event.width, height=event.height)
            self.canvas.config(scrollregion=(0, 0, self.current_photo.width(), self.current_photo.height()))


    def on_resize(self, event):
        # Update the size and position of the image on the canvas
        # Replace 'image_id' with the actual ID of the image on the canvas
        canvas.coords('image_id', 0, 0, event.width, event.height)

    def update_canvas(self):
        if self.current_photo:
            self.canvas.config(width=self.current_photo.width(), height=self.current_photo.height())
            self.canvas.delete("all")
            self.canvas.create_image(0, 0, anchor="nw", image=self.current_photo)


    def bind_mouse_events(self):
        if self.canvas:
            self.canvas.bind("<ButtonPress-1>", self.on_button_press)
            self.canvas.bind("<B1-Motion>", self.on_move_press)

    def bound_panning(self):
        if self.current_photo:
            image_width = self.current_photo.width() * self.scale_factor
            image_height = self.current_photo.height() * self.scale_factor
            canvas_width = self.canvas.winfo_width()
            canvas_height = self.canvas.winfo_height()

            # Allow panning beyond the image boundaries by a certain factor of the canvas size
            pan_limit_x = canvas_width * 5  # 50% of the canvas width
            pan_limit_y = canvas_height * 5  # 50% of the canvas height

            self.pan_x = min(pan_limit_x, max(self.pan_x, -image_width + canvas_width - pan_limit_x))
            self.pan_y = min(pan_limit_y, max(self.pan_y, -image_height + canvas_height - pan_limit_y))
        else:
            self.pan_x = 0
            self.pan_y = 0

    def reset_pan(self):
        self.pan_x = self.pan_y = 0

    def reset_scale(self):
        self.scale_factor = 1.0

    def start_timer(self):
        if self.timer:  # If a timer is already running
            self.timer.cancel()  # Cancel the existing timer
        h = int(h_time_entry.get())
        m = int(m_time_entry.get())
        s = int(s_time_entry.get())
        self.total_seconds = h * 3600 + m * 60 + s
        if self.total_seconds > 10:  # Only start the timer if the total time is greater than 10 seconds
            self.start_time = time.time()
            self.countdown()
        else:
            print("The total time must be greater than 10 seconds to start the timer.")

    # Update the countdown method to correctly display hours, minutes, and seconds
    def countdown(self):
        if self.total_seconds > 0:
            self.remaining_time = self.total_seconds  # Calculate remaining time at the start of each tick
            hours, remainder = divmod(self.total_seconds, 3600)
            minutes, seconds = divmod(remainder, 60)
            print(f"Time left: {hours}:{minutes}:{seconds}")
            timer.configure(text=f"{int(hours):02d}:{int(minutes):02d}:{int(seconds):02d}")
            self.timer = threading.Timer(1.0, self.countdown)
            self.timer.start()
            self.total_seconds -= 1
        else:
            print("Bzzzt! The countdown is at zero seconds!")
            self.pick_image()  # Call the pick_image method when the countdown reaches zero
            self.start_timer()  # Restart the timer

    def toggle_timer(self):
        if self.timer:
            self.stop_timer()
        else:
            self.resume_timer()

    def stop_timer(self):
        if self.timer:
            self.timer.cancel()
            self.timer = None
            self.stop_time = time.time()

    def resume_timer(self):
        if self.remaining_time > 0 and self.stop_time:
            self.total_seconds = self.remaining_time
            self.countdown()

    def select_all(self, event):  # Add 'event' as a second argument
        # select text after 50ms
        event.widget.after(50, lambda: event.widget.select_range(0, 'end'))

    def create_button(master, text, command):
        return customtkinter.CTkButton(master=master, text=text, command=command, bg_color="#909090")
    
    
    def toggle_ui(self):
        if ui_elements[0].winfo_viewable():
            for element in ui_elements:
                element.grid_info_store = element.grid_info()  # Store the grid options
                element.grid_forget()
        else:
            for element in ui_elements:
                grid_info = element.grid_info_store
                # Use the stored grid options
                element.grid(row=grid_info["row"], column=grid_info["column"], sticky=grid_info.get("sticky", ""))
    
    def toggle_median_filter(self):
        if self.median_filter_applied:
            # If the median filter is currently applied, revert it to the original
            self.current_image = self.original_image_median.copy()
            self.median_filter_button.configure(fg_color="#2D2D2D")
        else:
            # Otherwise, apply the median filter
            # Save the original image before applying the filter
            self.original_image_median = self.current_image.copy()

            # Convert the image to RGB mode if it's not already
            if self.current_image.mode != 'RGB':
                self.current_image = self.current_image.convert('RGB')

            # Split the image into individual color channels
            r, g, b = self.current_image.split()

            # Convert each color channel to an array and apply the median filter
            r_filtered = Image.fromarray(median_filter(np.array(r), size=4))
            g_filtered = Image.fromarray(median_filter(np.array(g), size=4))
            b_filtered = Image.fromarray(median_filter(np.array(b), size=4))

            # Recombine the filtered color channels into one image
            self.current_image = Image.merge("RGB", (r_filtered, g_filtered, b_filtered))

            # Change the color of the median filter button
            self.median_filter_button.configure(fg_color="dark grey")

        self.median_filter_applied = not self.median_filter_applied  # Toggle the flag
        self.current_photo = ImageTk.PhotoImage(self.current_image)
        self.display_image()
            

    def toggle_posterization(self):
        if self.is_posterized:
            # If the image is currently posterized, revert it to the original
            self.current_image = self.original_image_posterize.copy()
            self.posterize_button.configure(fg_color="#2D2D2D")
        else:
            # Otherwise, apply the posterization effect
            # Save the original image before posterizing
            self.original_image_posterize = self.current_image.copy()
            self.current_image = self.current_image.convert('P', palette=Image.ADAPTIVE, colors=8)
            # Change the color of the median filter button
            self.posterize_button.configure(fg_color="dark grey")

        self.is_posterized = not self.is_posterized  # Toggle the flag
        self.current_photo = ImageTk.PhotoImage(self.current_image)
        self.display_image()  # Update the displayed image


    
    def toggle_grayscale(self):
        if self.is_grayscale:
            # If the image is currently grayscale, revert it to the original
            self.current_image = self.original_image_grayscale.copy()
            self.grayscale_button.configure(fg_color="#2D2D2D")
        else:
            # Otherwise, apply the grayscale effect
            # Save the original image before greyscaling
            self.original_image_grayscale = self.current_image.copy()
            self.current_image = ImageOps.grayscale(self.current_image)
            # Change the color of the grayscale button
            self.grayscale_button.configure(fg_color="dark grey")

        self.is_grayscale = not self.is_grayscale  # Toggle the flag
        self.current_photo = ImageTk.PhotoImage(self.current_image)
        self.display_image()  # Update the displayed image

# Initialize the ImageViewer
viewer = ImageViewer()

# Initialize the Tkinter application
app = customtkinter.CTk()
app.geometry("1920x1080")
app.title("Image Viewer")
app.attributes('-topmost', True)  # This line makes the window stay on top
# Change the background color to a dark color
app.configure(bg="#2D2D2D")

# Create a frame for the buttons
button_frame = customtkinter.CTkFrame(master=app, bg_color="transparent")
button_frame.grid(row=1, column=0, padx=5, pady=5)

button_frame2 = customtkinter.CTkFrame(master=app, bg_color="transparent")
button_frame2.grid(row=0, column=0, padx=5, pady=5)

# Create a list to hold all the UI elements
ui_elements = []
ui_elements.append(button_frame)

# Configure the grid to make the columns expand
for i in range(7):  # Replace 13 with the number of columns in your grid
    button_frame.grid_columnconfigure(i, weight=1)
for i in range(7):
    button_frame2.grid_columnconfigure(i, weight=1)

def create_button_with_grid(master, text, command, row, column, padx=5):
    button = customtkinter.CTkButton(master=master, text=text, command=command, fg_color="#2D2D2D")
    button.grid(row=row, column=column, padx=padx)
    return button

# UI elements

# frame1
select_folder_button = create_button_with_grid(button_frame, "Folder", viewer.select_folder, 0, 0)
view_button = create_button_with_grid(button_frame, "New", viewer.pick_image, 0, 1)
go_back_button = create_button_with_grid(button_frame, "Back", viewer.go_back, 0, 2)
flip_button = create_button_with_grid(button_frame, "Flip", viewer.flip_horizontal, 0, 3)
rotate_button = create_button_with_grid(button_frame, "Rotate", viewer.rotate_image, 0, 4)
reset_button = create_button_with_grid(button_frame, "Reset", viewer.reset_image, 0, 5)
viewer.grayscale_button = create_button_with_grid(button_frame, "Grayscale", viewer.toggle_grayscale, 0, 6)
viewer.median_filter_button = create_button_with_grid(button_frame, "Median Filter", viewer.toggle_median_filter, 0, 7)
viewer.posterize_button = create_button_with_grid(button_frame, "Toggle Posterization", viewer.toggle_posterization, 0, 8)

# frame2
timer = customtkinter.CTkLabel(master=button_frame2, text="00:00:00", font=("Arial", 20))
timer.grid(row=0, column=0, padx=5)

def create_time_entry(master, row, column):
    time_entry = customtkinter.CTkEntry(master=master, width=50)
    time_entry.insert(0, '00')
    time_entry.bind('<FocusIn>', viewer.select_all)
    time_entry.grid(row=row, column=column, padx=5)
    return time_entry

# Use the function to create the time entries
h_time_entry = create_time_entry(button_frame2, 0, 2)
m_time_entry = create_time_entry(button_frame2, 0, 3)
s_time_entry = create_time_entry(button_frame2, 0, 4)

start_timer_button = create_button_with_grid(button_frame2, "Start", viewer.start_timer, 0, 5)
stopresume_timer_button = create_button_with_grid(button_frame2, "Stop/resume", viewer.toggle_timer, 0, 1)
toggle_ui_button = create_button_with_grid(button_frame2, "UI", viewer.toggle_ui, 0, 6)

# Toggle UI button
toggle_ui_button = create_button_with_grid(button_frame2, "UI", viewer.toggle_ui, 0, 6)

# canvas
app.grid_rowconfigure(2, weight=1)
app.grid_columnconfigure(0, weight=1)

canvas = customtkinter.CTkCanvas(app, bg="#808080", highlightthickness=1, highlightbackground="black")
canvas.grid(row=2, column=0, sticky='nsew', columnspan=3)

viewer.canvas = canvas
canvas.bind("<Configure>", viewer.on_resize)

app.mainloop()