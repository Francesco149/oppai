osu! pp advanced inspector (oppai) is a difficulty and pp calculator for osu! 
standard beatmaps. It works on any map, even unsubmitted ones and all you have
to do is supply it with the map's .osu file.

# Getting started
If you don't feel like using the command line, you can hop on the 
[shigetora chat](https://www.twitch.tv/shigetora) and type
`!oppai url_to_beatmap` followed by the parameters. But remember, you won't get
the full output which contains much more useful info than just pp! So I 
recommend spending 1 minute downloading the tool and learning to use it from 
the command line.

Examples:
```
!oppai https://osu.ppy.sh/osu/37658
!oppai https://osu.ppy.sh/osu/37658 +HDHR
!oppai https://osu.ppy.sh/osu/37658 +HDHR 500x 1m
```

Demonstration usage video on windows and linux: 
[here](https://my.mixtape.moe/wasune.webm).

* If you are on arch linux, you can use the AUR packages 
[oppai](https://aur.archlinux.org/packages/oppai/) or 
[oppai-git](https://aur.archlinux.org/packages/oppai-git/) maintained by 
ammongit.
Otherwise, download the latest binaries for your OS from 
[here](https://github.com/Francesco149/oppai/releases), extract the archive 
and place the executable anywhere you like. Advanced users are free to add oppai
to their PATH to use it anywhere.
* Open cmd (or your favorite terminal emulator if you're on linux) and 
`cd /path/to/your/oppai/folder` (forward slashes might be backwards on 
windows)
* Type `./oppai` for a list of possible parameters.

UPDATE:
You can now pipe beatmaps to oppai from stdin, which means that you can download
a map on the fly and call oppai on it in one simple command.

Some linux examples of piping:
```bash
curl https://osu.ppy.sh/osu/37658 | ./oppai -
curl https://osu.ppy.sh/osu/37658 | ./oppai - +HDHR
curl https://osu.ppy.sh/osu/37658 | ./oppai - +HDHR 99% 600x 1m
```

Windows examples of piping (using powershell):
```powershell
(New-Object System.Net.WebClient).DownloadString("https://osu.ppy.sh/osu/37658") | ./oppai -
(New-Object System.Net.WebClient).DownloadString("https://osu.ppy.sh/osu/37658") | ./oppai - +HDHR
(New-Object System.Net.WebClient).DownloadString("https://osu.ppy.sh/osu/37658") | ./oppai - +HDHR 99% 600x 1m
```

NOTE: to obtain the beatmap url, just open the desired map's page in your 
browser, click on the desired difficulty and copy the url, then replace /b/ with
/osu/.

# Compiling from source (Linux)
```bash
git clone https://github.com/Francesco149/oppai.git
cd oppai
./build.sh
```

To cross compile, you can edit the build.sh and add, for example, ```-m32``` 
after g++ in CXX.

# Compiling from source (Windows)
You need to have git bash installed. The easiest way to get it is to install 
GitHub desktop.

You will also need visual studio. Any version should do. You don't even need the
IDE, you can even get the stand-alone compiler without all the bloat.

Open git bash and type:

```bash
git clone https://github.com/Francesco149/oppai.git
```

Now open a visual studio command prompt:
```bash
cd \path\to\oppai\win
build.bat
```

The executable will be found in the build directory.
