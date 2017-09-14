import os
import subprocess
import sys

def runCmd(arg):
    cmd = subprocess.Popen(arg)
    runcode = None
    while runcode is None:
        runcode = cmd.poll()

def apply_compress_for_dir(compressonator, tex_path, out_dir):
    files = [f for f in os.listdir(tex_path) if os.path.isfile(os.path.join(tex_path, f))]
    for f in files:
        print f
        extension = os.path.splitext(f)[1]
        nef_file = f.replace(extension, ".dds")
        runCmd([compressonator, "-fd", "BC3", "-miplevels", "100", os.path.join(tex_path, f),  os.path.join(out_dir, nef_file)])

def main():
    apply_compress_for_dir(sys.argv[1], sys.argv[2], sys.argv[3])

if __name__ == "__main__":
    main()