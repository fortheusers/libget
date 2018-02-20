import os, json, zipfile

def zipdir(path, ziph):
    for root, dirs, files in os.walk(path):        
        for file in files:
            if root == "." and (file == "info.json" or file == "icon.png"):
                continue
            ziph.write(os.path.join(root, file))

try:
    os.mkdir("zips")
except:
    pass

curdir = os.getcwd()
packages = {}
packages["packages"] = []

for package in os.listdir("packages"):
    if os.path.isfile("packages/" + package):
        continue
    zipf = zipfile.ZipFile("zips/" + package + ".zip", 'w', zipfile.ZIP_DEFLATED)
    os.chdir(curdir + "/packages/" + package)
    zipdir(".", zipf)
    zipf.close()

    # this line isn't confusing at all
    packages["packages"].append({"name": package})
    
    # if a info.json file exists, load properties from it
    if os.path.exists("info.json"):
        target = packages["packages"][-1]
        props = json.load(open("info.json", "r"))
        vals = ["title", "author", "category", "version", "description", "details", "url", "license"]
        for val in vals:
            if val in props:
                target[val] = props[val]
    
    os.chdir(curdir)
    
out = open("repo.json", "w")
json.dump(packages, out, indent=4)
out.close()
