import subprocess
        
        
def run_python_script(script_path):
    try:
        subprocess.run(['python', script_path], check=True)
        print(f"Script '{script_path}' executed successfully.")
    except subprocess.CalledProcessError as e:
        print(f"Error running script '{script_path}': {e}")
    except FileNotFoundError:
        print(f"Script '{script_path}' not found.")

# Usage:
script_path = "burner.py"   #file to run
run_python_script(script_path)
