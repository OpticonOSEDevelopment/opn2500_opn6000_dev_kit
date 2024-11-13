import os
from bs4 import BeautifulSoup

# Define the path for the Sources folder
sources_folder = os.path.abspath(os.path.join(os.getcwd(), '../../projects/applications/Examples/Sources'))

# Create the Sources sub-folder if it doesn't exist
if not os.path.exists(sources_folder):
    os.makedirs(sources_folder)

# Function to extract code snippets from .html file and save to .C file
def extract_code_snippet(file_name):
    # Read the HTML content
    try:
        with open(file_name, 'r', encoding='utf-8', errors='replace') as file:
            soup = BeautifulSoup(file, 'html.parser')
    except Exception as e:
        print(f"Error reading file {file_name}: {e}")
        return
    
    # Find all <pre> tags and select the last one
    pre_tags = soup.find_all('pre')
    
    if pre_tags:
        last_pre_tag = pre_tags[-1]  # Get the last <pre> tag
        
        # Extract and clean the text from the last <pre> tag
        code_snippet = last_pre_tag.get_text()
        
        # Clean up the snippet: replacing HTML-encoded characters
        code_snippet = code_snippet.replace('&#60', '<').replace('&#62', '>')
        
        # Create the output filename in the Sources folder (replace .html with .C)
        base_name = os.path.splitext(file_name)[0]
        output_filename = os.path.join(sources_folder, os.path.basename(base_name) + '.c')
        
        # Save the cleaned code snippet to a .C file in the Sources folder
        with open(output_filename, 'w', encoding='utf-8') as output_file:
            output_file.write(code_snippet)
        print(f'Saved: {output_filename}')
    else:
        print(f"No <pre> tag found in {file_name}")

# Iterate through all .html files in the current directory
for file_name in os.listdir(os.getcwd()):
    if file_name.endswith('.html'):
        extract_code_snippet(file_name)
