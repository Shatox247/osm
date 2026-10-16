# Open Sound Meter Fork with SPL Data logging capability 
Got annoyed by waiting for OSM getting SPL logging functionality. Therefore here is a version with exactly that. It outputs a csv with the contents from all visible meters, logged every second.
In the file menu on the top bar is a button to start/stop logging, the path to the current log storage location and a button to select a new destination.
I also quickly vibe coded a small viewer for the log csvs that plots the graphs and lets you choose which sources shall be displayed, available at https://github.com/Shatox247/SPL_Log_Viewer .

If you don't hate yourself enough to deal with Qt5 compilation (apparently Pavel is working on a port to Qt6) you can use the windows application compiled by an automatic github action. (repo > actions > latestRun > artefacts > osm windows zip).
Obviously Windows will warn you when starting the application, since it isn't signed or anything. But since its opensource, you can just look up the diff between the main osm repo and this fork and judge for yourself.

# Open Sound Meter
Crossplatform measurement software for tuning sound systems.

**Supported systems:** macOS, Windows, Linux

Current version: v1.4.1

[Project page](https://opensoundmeter.com/)

![](https://private-user-images.githubusercontent.com/683461/391698445-324038eb-a325-4951-9c29-ee404bebbf19.png?jwt=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJnaXRodWIuY29tIiwiYXVkIjoicmF3LmdpdGh1YnVzZXJjb250ZW50LmNvbSIsImtleSI6ImtleTUiLCJleHAiOjE3MzkxMzYyMTcsIm5iZiI6MTczOTEzNTkxNywicGF0aCI6Ii82ODM0NjEvMzkxNjk4NDQ1LTMyNDAzOGViLWEzMjUtNDk1MS05YzI5LWVlNDA0YmViYmYxOS5wbmc_WC1BbXotQWxnb3JpdGhtPUFXUzQtSE1BQy1TSEEyNTYmWC1BbXotQ3JlZGVudGlhbD1BS0lBVkNPRFlMU0E1M1BRSzRaQSUyRjIwMjUwMjA5JTJGdXMtZWFzdC0xJTJGczMlMkZhd3M0X3JlcXVlc3QmWC1BbXotRGF0ZT0yMDI1MDIwOVQyMTE4MzdaJlgtQW16LUV4cGlyZXM9MzAwJlgtQW16LVNpZ25hdHVyZT1mODdmNmVmYmI0NTEyM2Y3YTg2MjZiMzc2MzE4NmM0M2MwOTdkYzc4ZTdlNDRlMWZkOGVjYTZkZWM1M2Y3NDczJlgtQW16LVNpZ25lZEhlYWRlcnM9aG9zdCJ9.w1SSK4YF7kJ7cd3dIp8xrYIv-o0dCXNddjCdjO2vxYk)

Qt5.15 C++17

## Additional functionality of this fork: SPL logging

Use **File > Start SPL logging** (or the side menu) to write the numerical values
currently shown by the SPL meters to `SPL_Log_DATE_TIME.csv` per default in the Windows
user application-data directory on the C: drive (for example,
`C:/Users/<user>/AppData/Local/opensoundmeter/OpenSoundMeter/SPL_Log_2026-10-25_21-51-48.csv`).
The location to write to is displayed in the field below the start/stop button, below that is a button to set a new target directory.
Samples are written at a fixed one-second interval. Stop logging from the same
menu. 
The csv contains a leading column with the time stamp followed by a column for each meter in all visible numeric graphs.

## SAST Tools

[PVS-Studio](https://pvs-studio.com/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.
