#!/usr/bin/python
import os, json, zipfile, time, datetime, hashlib, re

print("Content-type: text/html\n\n")

SCREEN_REGEX = re.compile("^screen[1-9].png$")

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
    screen_count = 0

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
                if root == "." and (file == "manifest.install" or file == "icon.png" or file == "info.json" or file == "screen.png" or file == ".deletetoupdate" or SCREEN_REGEX.match(file)):
                    continue
                relPath = os.path.join(root, file)[2:]
                relPath.replace("\\", "/")
                if relPath in existing:
                    manifest += existing[relPath]
                else:
                    manifest += "U: %s\n" % relPath
        manifest_file = open("manifest.install", "w")
        manifest_file.write(manifest)
        manifest_file.close()

        print("Zipping %s...<br>" % package)
        print(".speak #hbas-updates WIIU : https://apps.fortheusers.org/switch/%s <br>" % package)
        zipdir(".", zipf)
        zipf.close()

    cached_info = {}
    needsCaching = True

    try:
        del_to_update_file = open(".deletetoupdate", "r")
        cached_info = json.load(del_to_update_file)
        del_to_update_file.close()
        filesize = cached_info["filesize"]
        folder_size = cached_info["extracted"]
        binary = cached_info["binary"]
        updated = cached_info["updated"]
        mdhex = cached_info["md5"]
        screen_count = cached_info["screens"]
        needsCaching = False
    except:
        # data in .deletetoupdate wasn't useful, let's calculate that info now

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
                if SCREEN_REGEX.match(file):
                    screen_count += 1

        # Date last updated (assumption is that if the app is updated the info.json would be)
        updated = time.strftime('%d/%m/%Y', time.gmtime(os.path.getmtime(curdir + "/packages/" + package + "/info.json")))

        #md5 of package zip
        filehash = hashlib.md5()
        filehash.update(open(curdir + "/zips/" + package + ".zip").read())
        mdhex = filehash.hexdigest()
        
        cached_info = { "filesize": filesize, "extracted": folder_size, "binary": binary, "updated": updated, "md5": mdhex, "screens": screen_count}

    # this line isn't confusing at all (additional info makes it less so)
    packages["packages"].append({"name": package, "filesize": filesize, "updated": updated, "extracted": folder_size, "binary": binary, "md5": mdhex, "screens": screen_count})

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

    if needsCaching:    # no deletetoupdate loaded, we need to make one
        del_to_update = open(".deletetoupdate", 'w')
        json.dump(cached_info, del_to_update, sort_keys=True, indent=4, separators=(',', ': '))
        del_to_update.close()
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

out = open("repo.json", "w")
json.dump(packages, out, indent=4)
out.close()

print("All Done!<br>")
