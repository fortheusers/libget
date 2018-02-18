import os, json, zipfile

def zipdir(path, ziph):
    for root, dirs, files in os.walk(path):
        for file in files:
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
    os.chdir(curdir)

    # this line isn't confusing at all
    packages["packages"].append({"name": package})

out = open("repo.json", "w")
json.dump(packages, out)
out.close()
