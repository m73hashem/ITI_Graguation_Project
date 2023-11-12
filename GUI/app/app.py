from tkinter import *
from tkinter import messagebox, filedialog
from tkinter import PhotoImage
from tkinter import StringVar
from time import strftime
import webbrowser
import subprocess
import time
import os
import sys
import requests
from urllib.parse import urljoin
import hashlib
from datetime import datetime
from tkinter import messagebox

# Initialize the hash of the last downloaded version and the file name
last_file_hash = None
last_file_name = None

# Function to update the label with a message
def update_label_text(message):
    update_label.config(text=message)
    window.update_idletasks()

# Function to check for updates and print messages accordingly
def check_for_updates():
    global last_file_hash, last_file_name
    web_page_url = "https://team4gp.pythonanywhere.com/"  # Replace with the actual URL of the web page
    save_subdirectory = "app"
    save_directory = os.path.join(os.getcwd(), save_subdirectory)  # Save in the 'app' subdirectory

    response = requests.get(web_page_url)
    if response.status_code == 200:
        # Parse the HTML content
        html_content = response.text

        # Search for the download link in the HTML content
        download_link_start = html_content.find('<a href="') + len('<a href="')
        download_link_end = html_content.find('" id="downloadLink" download', download_link_start)
        relative_download_url = html_content[download_link_start:download_link_end]

        # Create the absolute URL by joining the relative URL with the base URL
        absolute_download_url = urljoin(web_page_url, relative_download_url)

        # Calculate the hash of the remote file
        response = requests.get(absolute_download_url)
        remote_file_hash = hashlib.sha256(response.content).hexdigest()

        if last_file_hash is None or remote_file_hash != last_file_hash:
            # A new version is available, ask the user if they want to download it
            user_response = messagebox.askquestion("Update Available", "A new version is available. Do you want to download it?")
            if user_response == "yes":
                file_extension = os.path.splitext(absolute_download_url)[-1]
                timestamp = datetime.now().strftime("%Y%m%d%H%M%S")
                file_name = f"downloaded_file_{timestamp}{file_extension}"
                save_path = os.path.join(save_directory, file_name)

                # Send an HTTP GET request to the absolute download URL
                response = requests.get(absolute_download_url, stream=True)

                if response.status_code == 200:
                    # Save the file with a new name in the 'download' subdirectory
                    with open(save_path, 'wb') as file:
                        for chunk in response.iter_content(chunk_size=8192):
                            file.write(chunk)
                    messagebox.showinfo("Download Complete", f"File updated and saved to {save_path}")

                    # Update the last file hash and name
                    last_file_hash = remote_file_hash
                    last_file_name = file_name
                else:
                    messagebox.showerror("Download Failed", f"Failed to download file. Status code: {response.status_code}")
            else:
                update_label_text("User chose not to download the new version.")
        else:
            update_label_text("File is up to date.")
    else:
        update_label_text(f"Failed to fetch the web page. Status code: {response.status_code}")

# Function to run when the "Run" button is clicked
def run_script():
    try:
        script_path = "run_script.py"  # Replace with the path to your Python script
        python_executable = sys.executable
        subprocess.run([python_executable, script_path], check=True)
        print(f"Script '{script_path}' executed successfully.")
    except subprocess.CalledProcessError as e:
        print(f"Error running script '{script_path}': {e}")
    except FileNotFoundError:
        print(f"Script '{script_path}' not found.")

# Create a Tkinter window
window = Tk()
window.title("Update Checker app")
window.geometry("650x500")
# Create and configure a label for displaying messages

update_text = Text(window, wrap=WORD, height=2, width=40, state=DISABLED, bg="white")
update_text.place(x=140, y=20)

# Function to update the text widget with a message
def update_text_content(message):
    update_text.config(state=NORMAL)
    update_text.delete(1.0, END)
    update_text.insert(END, message)
    update_text.config(state=DISABLED)

# ... (continue with your existing code)

# Function to update the label with a message
def update_label_text(message):
    update_label.config(text=message)
    update_text_content(message)

# ... (continue with your existing code)

# Create and configure a label for displaying messages
update_label = Label(window, text="", wraplength=400)
update_label.place(x=200, y=400)

# Create and configure the "Check for Updates" button
check_button = Button(window, text="Check for Updates", command=check_for_updates, font=("Comic Sans MS", 14, "bold"), fg="#341957")
check_button.place(x=50, y=80)

# Create and configure the "Run" button
run_button = Button(window, text="Run Script", command=run_script, font=("Comic Sans MS", 14, "bold"), fg="#341957")
run_button.place(x=450, y=80)

photo = PhotoImage(file="model-hyper-configurator.png")
photo = photo.subsample(1, 1)
Label(window, image=photo).place(x=10, y=150)

# Function to periodically check for updates in the background
def periodic_check():
    check_for_updates()
    window.after(3000, periodic_check)  # Schedule the function to run every 5 minutes (300 seconds)

# Start periodic update checking in the background
window.after(0, periodic_check)

# Start the Tkinter main loop
window.mainloop()
