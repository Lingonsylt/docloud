import os
import sqlite3
from os.path import join
from shutil import copy, rmtree, copytree
import sys
if sys.version_info < (3, 3, 0):
    FileNotFoundError = OSError

def package(bits="x64"):
    pkg_path = join(os.path.dirname(__file__), "pkg")
    print("Creating package at: %s" % pkg_path)
    service_path = join(os.path.dirname(__file__), "service")
    shared_path = join(os.path.dirname(__file__), "shared")
    docloudext_path = join(os.path.dirname(__file__), "docloudext")

    try:
        rmtree(pkg_path)
    except FileNotFoundError:
        pass
    os.mkdir(pkg_path)
    copy(join(shared_path, "sqlite3", "sqlite3.dll"), pkg_path)
    copy(join(shared_path, "schema.sql"), pkg_path)
    for file in os.listdir(join(shared_path, bits)):
        copy(join(shared_path, bits, file), pkg_path)
    copy(join(docloudext_path, "docloudext.dll"), pkg_path)
    copy(join(service_path, "docloud-svc.exe"), pkg_path)

    print("Package created at: %s" % pkg_path)
    return pkg_path

def install(pkg_path, reinstall=False, kill_explorer=False):
    import winreg
    print("Installing package from: %s" % pkg_path)
    install_path = os.environ.get("INSTALL_DIR", join(os.path.dirname(__file__), "install"))
    if reinstall:
        try:
            rmtree(install_path)
        except FileNotFoundError:
            pass
    if kill_explorer:
        os.system("taskkill /f /im explorer.exe")
    copytree(pkg_path, install_path)
    db_path = join(install_path, "db.sqlite")
    docloud_key = winreg.CreateKey(winreg.HKEY_LOCAL_MACHINE, "Software\\Docloud\\Docloud")
    try:
        db_name, no = winreg.QueryValueEx(docloud_key, "database")
        if reinstall:
            try:
                os.remove(db_name)
            except FileNotFoundError:
                pass
            winreg.SetValueEx(docloud_key, "database", 0, winreg.REG_SZ, db_path)
            db_name = db_path
        else:
            print("Docloud is already installed!")
            return
    except FileNotFoundError:
        winreg.SetValueEx(docloud_key, "database", 0, winreg.REG_SZ, db_path)
        db_name = db_path

    if reinstall:
        try:
            os.remove(db_name)
        except FileNotFoundError:
            pass
    conn = sqlite3.connect(db_name)
    with open(join(pkg_path, "schema.sql")) as schema_f:
        conn.executescript(schema_f.read())

    os.system("regsvr32 %s" % join(install_path, "docloudext.dll"))
    if kill_explorer:
        os.system("explorer.exe")

    print("Starting service:")
    os.system(join(install_path, "docloud-svc.exe"))

def uninstall():
    print("Uninstalling docloud")
    import winreg
    docloud_key = winreg.CreateKey(winreg.HKEY_LOCAL_MACHINE, "Software\\Docloud\\Docloud")
    try:
        db_name, no = winreg.QueryValueEx(docloud_key, "database")
        try:
            print("Deleting database %s" % db_name)
            os.remove(db_name)
        except FileNotFoundError:
            pass
    except FileNotFoundError:
        pass
    winreg.DeleteKey(docloud_key, "")

args = sys.argv[1:]
if "uninstall" in args:
    uninstall()
if "package" in args:
    pkg_path = package()
    if "install" in args:
        install(pkg_path,
                reinstall="--reinstall" in args or "-r" in args,
                kill_explorer="--killexplorer" in args or "-k" in args)
elif "install" in args:
    install(".")
elif "uninstall" not in args:
    print("Options are: package, install, uninstall, --killexplorer (-k), --reinstall (-r)")