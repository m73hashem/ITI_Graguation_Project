from tkinter import *
from tkinter import messagebox, filedialog
from time import strftime
from PIL import Image
from tkinter import filedialog
import webbrowser
import subprocess
import time
import os
import hashlib
import json
import base64
from tkinter import StringVar
import sys
import requests
from urllib.parse import urljoin
from datetime import datetime
from tkinter import messagebox
#import app
img = None
img2 = None

def cheaking_button():
    file_path = "app/app.py"  # Replace with the actual path to your Python script file
    try:
        subprocess.run(["python", file_path], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error running the Python script: {e}")
    except FileNotFoundError:
        print("Python interpreter not found. Make sure it is in your system's PATH.")


def on_upload_click():
    confirmed = messagebox.askyesno("Confirmation", "Are you sure you want to Upload new version ?")
    if confirmed:
        webbrowser.open("https://team4gp.pythonanywhere.com/")
        time.sleep(3)
        subprocess.call('taskkill /F /IM msedge.exe /T', shell=True)  # Close Microsoft Edge browser
        time.sleep(5)
        file_path = "C:\\Users\\PROCESSOR\\Downloads\\admin (2).txt"  # Replace with the actual path to your text file
        os.startfile(file_path)
        messagebox.showinfo("Upload Bootloader", "Code uploaded successfully!")

def show_main_interface():
    global img, img2
    # Create the main window
    window = Toplevel()
    window.title("Bootloader and Updates")
    window.geometry("1024x643")
    window.config(bg="#87a0c9")

    # Load images with PhotoImage and keep references
    img_path = "FOTA-1024x643-1.png"
    img = PhotoImage(file=img_path)
    #img = img.subsample(2,2)
    img2_path = "iti-logo.png"
    img2 = PhotoImage(file=img2_path)
    img2 = img2.subsample(8,8)
    Label(window, image=img).place(x=5, y=0)
    Label(window, image=img2).place(x=5, y=560)

    bootloader_label = Label(window, text="Firmware Over The Air", font=("bold" "80" "Roman"), fg="black")
    bootloader_label.place(x=820, y=615)


    upload_button = Button(window, text="Get Updates", command=cheaking_button, pady=20, padx=35, bg="white",font=("calibri", 16,"bold"))
    upload_button.place(x=40, y=180)

    #cheak_button =Button(window, text="Check for Updates",command=cheaking_button)
    #cheak_button.place(x=40, y=230)
    #file_path_entry = Entry(window, bg="white", width=50, fg="#06021f")
    #file_path_entry.place(x=140, y=70)





    # Create three choice lists
    car_make_label = Label(window, text="Car Model" ,font=('calibri', 16, 'bold'),bg="#292E34",fg="#e8e9ed")
    car_make_label.place(x=590 ,y=20 )
    car_make_var = StringVar(window)
    car_make_var.set("Select Car Model")
    car_make_optionmenu =OptionMenu(window, car_make_var, "N_1 : _Toyota_ ", "N_2 : _Honda_ ", "N_3 : _volkswagen_" ,"N_4 : _Mercedes_","N_5 : _Mitsubishi_")
    car_make_optionmenu.place(x=570 ,y=70 )

    car_year_label = Label(window, text="Car Year",font=('calibri', 16, 'bold'),bg="#292E34",fg="#e8e9ed")
    car_year_label.place(x=750 ,y=20 )
    car_year_var =StringVar(window)
    car_year_var.set("Select Car Year")
    car_year_optionmenu =OptionMenu(window, car_year_var, "")   #Euphemia
    car_year_optionmenu.place(x=730,y=70)

    car_model_label =Label(window, text="Car Target",font=('calibri', 16, 'bold'),bg="#2F343A",fg="#e8e9ed")
    car_model_label.place(x=900 ,y=20 )
    car_model_var =StringVar(window)
    car_model_var.set("Select Car Target")
    car_model_optionmenu = OptionMenu(window, car_model_var, "")
    car_model_optionmenu.place(x=880 ,y=70 )

    # Update car model choices based on the selected car make
    def update_car_model_choices(*args):
        selected_year = car_year_var.get()
        if selected_year == "2020" or "2021" or "2022" or "2023":
            car_model_optionmenu['menu'].delete(0, 'end')
            car_model_optionmenu['menu'].add_command(label="[STM32F401CCU6]", command=lambda: car_model_var.set("[STM32F401CCU6]"))
            car_model_optionmenu['menu'].add_command(label="[STM32F103C6T6]", command=lambda: car_model_var.set("[STM32F103C6T6]"))
            car_model_optionmenu['menu'].add_command(label="[ _ TIVAC-1 _ ]", command=lambda: car_model_var.set("[ _ TIVAC-1 _ ]"))
  

        car_model_var.set("Select Car Target")

    car_year_var.trace("w", update_car_model_choices)

    # Update car year choices based on the selected car model
    def update_car_year_choices(*args):
        selected_make = car_make_var.get()
        if selected_make == "Toyota" or "Honda" or "Ford":
            car_year_optionmenu['menu'].delete(0, 'end')
            car_year_optionmenu['menu'].add_command(label="N_1 : [ 2020 ]", command=lambda: car_year_var.set("N_1 : [ 2020 ]"))
            car_year_optionmenu['menu'].add_command(label="N_2 : [ 2021 ]", command=lambda: car_year_var.set("N_2 : [ 2021 ]"))
            car_year_optionmenu['menu'].add_command(label="N_3 : [ 2022 ]", command=lambda: car_year_var.set("N_3 : [ 2022 ]"))
            car_year_optionmenu['menu'].add_command(label="N_4 : [ 2023 ]", command=lambda: car_year_var.set("N_4 : [ 2023 ]"))
    

    car_model_var.set("Select Car Target")

    car_make_var.trace("w", update_car_year_choices)

    
    def on_browse_click():
        file_path = filedialog.askopenfilename(filetypes=[("Python files", "*.py")])
        if file_path:
            try:
                with open(file_path, 'r') as file:
                    script_contents = file.read()
                    exec(script_contents)
            except FileNotFoundError:
                print("File not found or path is incorrect.")
            except IOError:
                print("Error reading the file.")

            # Update the entry widget with the selected file path
            file_path_entry.delete(0, END)
            file_path_entry.insert(0, file_path)
    

    browse_button =Button(window, text="BROWSE", command=on_browse_click, bg="white" ,pady=20,padx=35,font=("calibri", 16,"bold"))
    browse_button.place(x=40 ,y=40 )

#########Add A clock #########


    def time():
        string = strftime('%H:%M:%S %p')
        lbl.config(text=string)
        lbl.after(1000, time)
 
 
# Styling the label widget so that clock
# will look more attractive
    lbl = Label(window, font=('calibri', 10, 'bold'),
            background='purple',
            foreground='white')
 
# Placing clock at the centre
# of the tkinter window
    lbl.pack(anchor=NW)
    time()

  

    window.mainloop()
    # Run the main window's event loop


def start_window():
    global username_entry, password_entry, confirm_password_entry, login_window, signup_window ,show_main_interface

    start_window = Tk()
    start_window.title("Start Window")
    start_window.geometry("650x400")

    photo = PhotoImage(file="model-hyper-configurator.png")
    photo = photo.subsample(1, 1)
    Label(start_window, image=photo).place(x=5, y=5)

    label1 = Label(start_window, text="FOTA", font=("Comic Sans MS", 24,"bold"),fg="#341957")
    label1.pack()

    label2 = Label(start_window, text="from OEM directly to the driver", font=("Comic Sans MS", 24,"bold"),fg="#341957")
    label2.pack()

    login_button = Button(start_window, text="Log in", font=("Helvetica", 14), command=login_window,bg="#791df2",fg="white")
    login_button.place(x=150, y=350)

    signup_button = Button(start_window, text="Sign up", font=("Helvetica", 14), command=signup_window,bg="#791df2",fg="white")
    signup_button.place(x=450, y=350)

    start_window.mainloop()

# Function to create login window
def login_window():
    global username_entry, password_entry, login_window ,show_main_interface

    login_window = Tk()
    login_window.title("Login")
    login_window.geometry("350x300")
 
    
    label=Label(login_window, text="FOTA" , font=("Comic Sans MS", 20 , "bold"),fg="#341957" )
    label.place(x=120,y=10)
    label=Label(login_window, text="Log in to your existing account" , font=("Comic Sans MS", 14 ,"bold"),fg="#341957" )
    label.place(x=30,y=60)
    

    username_entry = Entry(login_window, width=30)
    username_entry.place(x=150,y=125)

    password_entry = Entry(login_window, width=30, show='*')
    password_entry.place(x=150,y=175)

    Label(login_window, text="User Name :", font=("Euphemia",14,"bold") , fg="#3d386b").place(x=10,y=120)
    Label(login_window, text="Password  :", font=("Euphemia",14,"bold"), fg="#3d386b").place(x=10,y=170)

    login_button = Button(login_window, text="Log in", command=on_login_click, padx=20, bg="light green", fg="#396ec4",font=("Helvetica", 14))
    login_button.place(x=110,y=220)

    
    login_window.mainloop()

# Function to create signup window
def signup_window():
    global username_entry, password_entry, confirm_password_entry, signup_window ,show_main_interface

    signup_window = Tk()
    signup_window.title("Signup")
    signup_window.geometry("300x300")

    label=Label(signup_window , text="FOTA", font=("Comic Sans MS", 20),fg="#341957" )
    label.place(x=120,y=10)

    label=Label(signup_window , text="Creat a new account", font=("Comic Sans MS", 20),fg="#341957" )
    label.place(x=30,y=50)

    username_entry = Entry(signup_window, width=25)
    username_entry.place(x=140,y=115)

    password_entry = Entry(signup_window, width=25, show='*')
    password_entry.place(x=140,y=165)

    confirm_password_entry = Entry(signup_window, width=25, show='*')
    confirm_password_entry.place(x=140,y=205)

    Label(signup_window, text="User Name :", font=("Euphemia",14,"bold"), fg="#3d386b").place(x=0,y=110)
    Label(signup_window, text="Password   :", font=("Euphemia",14,"bold"), fg="#3d386b").place(x=0,y=160)
    Label(signup_window, text="Confirm Pass :", font=("Euphemia",14,"bold"), fg="#3d386b").place(x=0,y=200)

    signup_button = Button(signup_window, text="Sign up", command=on_signup_click, padx=20, bg="light green", fg="#396ec4",font=("Helvetica", 14))
    signup_button.place(x=100,y=240)

    signup_window.mainloop()
def on_signup_click():
    username = username_entry.get()
    password = password_entry.get()
    confirm_password = confirm_password_entry.get()

    if password != confirm_password:
        messagebox.showerror("Signup Failed", "Passwords do not match")
        return

    # Hash the password using a secure hashing algorithm
    hashed_password = hash_password(password)

    # Save the username and hashed password (and other user details) in a file
    save_user_info(username, hashed_password)

    signup_window.destroy()
    show_main_interface()

def hash_password(password):
    # Use a secure hashing algorithm (e.g., SHA-256) and add a salt to enhance security
    salt = b'some_random_salt'
    hashed_password = hashlib.pbkdf2_hmac('sha256', password.encode('utf-8'), salt, 100000)
    return base64.b64encode(hashed_password).decode('utf-8')

def save_user_info(username, hashed_password):
    user_info = {"username": username, "password": hashed_password}

    # Save the user information to a file (in JSON format, for simplicity)
    with open("user_info.json", "w") as file:
        json.dump(user_info, file)

def load_user_info():
    try:
        # Load user information from the file
        with open("user_info.json", "r") as file:
            user_info = json.load(file)
        return user_info
    except FileNotFoundError:
        return None


def on_login_click():
    user_info = load_user_info()

    if user_info is None:
        messagebox.showerror("Login Failed", "No user information found")
        return

    stored_username = user_info.get("username")
    stored_password = user_info.get("password")

    entered_username = username_entry.get()
    entered_password = password_entry.get()

    if entered_username == stored_username and verify_password(entered_password, stored_password):
        login_window.destroy()
        show_main_interface()
    else:
        messagebox.showerror("Login Failed", "Invalid username or password")

def verify_password(entered_password, stored_password):
    # Verify the entered password against the stored hashed password
    entered_password_hashed = hash_password(entered_password)
    return entered_password_hashed == stored_password

start_window()
