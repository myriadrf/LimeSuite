import os

# Change working directory to the one this file is in.
abspath = os.path.abspath(__file__)
dname = os.path.dirname(abspath)
os.chdir(dname)

directory_in_str = "apidoc/class"
directory = os.fsencode(directory_in_str)
    
for file in os.listdir(directory):
    filename = os.fsdecode(file)

    if filename.endswith(".rst"):
        with open(directory_in_str + "/" + filename, "a") as myfile:
            myfile.write("   :undoc-members:")
