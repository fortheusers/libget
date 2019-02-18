#!/usr/bin/python
import os, json, zipfile, time, datetime, hashlib

print("Content-type: text/html\n\n")

def zipdir(path, ziph):
    for root, dirs, files in os.walk(path):
        for file in files:
            if root == "." and (file == "icon.png" or file == "screen.png" or file == ".deletetoupdate"):
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
    if os.path.isfile("packages/" + package) or (len(package) > 0 and package[0] == "."):
        continue

    # if the .deletetoupdate file exists, don't compress
    # also if the zip doesn't exist already
    skipRebuild = os.path.exists(curdir + "/packages/%s/.deletetoupdate" % package) and os.path.exists(curdir + "/zips/%s.zip" % package)

    if not skipRebuild:
        zipf = zipfile.ZipFile("zips/" + package + ".zip", 'w', zipfile.ZIP_DEFLATED)

    os.chdir(curdir + "/packages/" + package)

    if not skipRebuild:

        # generate a manifest to go inside of this zip
        # pull in any existing manifest and only write U entries
        # omitted files
        existing = {}
        try:
            manifest_file = open("manifest.install", "r")
            for line in manifest_file:
                mode, name = line.split(": ")
                if mode == "U":
                    continue
                existing[name.strip()] = line
            manifest_file.close()
        except:
            pass

        manifest = ""
        for root, dirs, files in os.walk("."):
            for file in files:
                if root == "." and (file == "manifest.install" or file == "icon.png" or file == "info.json" or file == "screen.png" or file == ".deletetoupdate"):
                    continue
                relPath = os.path.join(root, file)[2:]
                if relPath in existing:
                    manifest += existing[relPath]
                else:
                    manifest += "U: %s\n" % relPath
        manifest_file = open("manifest.install", "w")
        manifest_file.write(manifest)
        manifest_file.close()

        print("Zipping %s...<br>" % package)
        zipdir(".", zipf)
        zipf.close()

    # Detail zip package size in KB's
    filesize = os.path.getsize(curdir + "/zips/" + package + ".zip")/1024

    # Detail extracted directory size  in KB's
    folder_size = 0
    for (root, dirs, files) in os.walk('.'):
        for file in files:
            fname = os.path.join(root, file)
            folder_size += os.path.getsize(fname)/1024

	# Include the nro name in json if exists
	binary = "none"
	for (root, dirs, files) in os.walk('.'):
		for file in files:
			if file.endswith(".nro"):
				binary = (root + "/" + file)[1:]


    # Date last updated (assumption is that if the app is updated the info.json would be)
    updated = time.strftime('%d/%m/%Y', time.gmtime(os.path.getmtime(curdir + "/packages/" + package + "/info.json")))

    #md5 of package zip
    filehash = hashlib.md5()
    filehash.update(open(curdir + "/zips/" + package + ".zip").read())
    mdhex = filehash.hexdigest()

    # this line isn't confusing at all (additional info makes it less so)
    packages["packages"].append({"name": package, "filesize": filesize, "updated": updated, "md5": mdhex, "extracted": folder_size,"binary": binary})

    # if a info.json file exists, load properties from it
    if os.path.exists("info.json"):
        target = packages["packages"][-1]
        props = json.load(open("info.json", "r"))
        vals = ["title", "author", "category", "version", "description", "details", "url", "license", "changelog"]
        for val in vals:
            if val in props and props[val]:
                target[val] = props[val]
            else:
                target[val] = "n/a"

    open(".deletetoupdate", 'a').close()

    os.chdir(curdir)

# do download counts for app and web for all packages
wiiu = False
targets = ["app", "web"]
root = "../"
if wiiu:
    targets = ["wiiu"]
    root = ""


try:
    for target in targets:
        # open all stats from the cron job'd stats json file for this target
        statsfile = open("%s../history/logs/%s/output.json" % (root, target), "r")
        stats = json.load(statsfile)
        statsfile.close()

        for package in packages["packages"]:
            if package["name"] in stats:
                if wiiu:
                    target = "all"
                package["%s_dls" % target] = stats[package["name"]]
except:
    print("Encountered an issue getting stats<br>")

zipf = zipfile.ZipFile("zips/icons.zip", 'w', zipfile.ZIP_DEFLATED)
# get all pngs out of the packages folder to add them to one zip
for package in os.listdir("packages"):
    if os.path.isfile("packages/" + package) or (len(package) > 0 and package[0] == "."):
        continue
    for file in os.listdir("packages/%s" % package):
        # only write these two pngs
        if file == "icon.png" or file == "screen.png":
            zipf.write(os.path.join("packages/%s" % package, file))
zipf.close()

out = open("repo.json", "w")
json.dump(packages, out, indent=4)
out.close()

print("All Done!<br>")
