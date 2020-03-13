##TimeAttackRacer Mid Term Review

Game Name: TAR

#Overview:

- To design and implement a 3D time trial racing game. The player should be able to drive around a track and make it through checkpoints in order, upon completing a race, the player would see his total race time, and if the best time was beaten, the new time becomes the record to beat.

##Initial Plan

Project Description: 

Arcade style split-screen 3D time attack racing game using Xbox controller as input device. Tracks made using mod kits featuring a closed circuit with checkpoints to determine progress in the game. 
Intent is to create a fully functional racing game with at least 1 car and 1 complete racetrack with checkpoints and a time tracking system.

#Project Must haves:
	-	Must have a working car than can be used to drive around the track
	-	Must have 1 working track at least
	-	Must have a checkpoint system to track time and record laps/checkpoints

#Stretch Goals:
	-	Split Screen MP
	-	Add a mini map for the level
	-	Adding a racing HUD (speedometer, time, checkpoint number)
	-	Adding multiple cars to choose from
	-	Adding multiple tracks to choose from
	-	Adding ghost car feature to race against the best time recorded
	-	Add UI for car select, track select and so on

#Super Stretch Goals if given the time:
	-	Create a Level Editor to utilize the mod kit pieces and create custom tracks (rapid prototype using dear ImGUI)
	-	Create procedural tracks using mod kit piece

##Featues Implemented:

	- Physics based car
	- Racetrack system with render mesh and collision meshes
		- Setup object loading for the render mesh and allow events to fire when collision meshes are required
		- Have the PhysXSystem handle the collision geometry generation and add to scene
	- Have waypoints/checkpoints you can cross in order along with lap tracking. They should store time between checkpoints and display lap times  
	- Support split screen views for up to 4 players. 
		- Implement a splitscreen system that supports viewport splits for 1,2,3,4 players
		- Be able to define if you prefer vertical or horizontal splits


#Timeline of Development:

#Week 1:

	- Cloning from Protogame3D and setting up new project for TAR
	- Project setup on Github using SourceTree as client
	- Finding mod kit track pieces to be able to design a racetrack

#Week 2:

	- Implement collisions using OBJ mesh files.
	- Tested and finalizeed the approach for collision mesh loading
	- Implemented Collision mesh loading using events in ObjectLoader class

#Week3:

	- Implemented Tier 1 racetrack with all collision meshes loading
	- Implemented ramps and height collision as static RBs
	- Began working on checkpoint system

#Week 4:

	- Implemented the checkpoint system
	- Added lap counting and per checkpoint and per lap time tracking

#Week 5:

	- Resolved performance issues with Rendering pipeline setup
		- Ensured driver issues were resolved
		- Picking the DX card always
		- Disabled VSync
	- Implemented Fixed Time steps for physics
		- Introduced a FixedUpdate to Game
		- Introduced deltaTimeCaching to update PhysX step atleast once per frame
	- Started implementation of split screen system

#Week 6:

	- Implemented a split screen system to take car camera and setup viewports based on how many controllers are connected
	- Resolved bugs with split screen system and multiple player controllers

#Week 7:

	- Resolving bugs with waypoint system
	- Re-architecting player Car to account for CarController, CarCamera and WaypointSystem updates

#Week 8:

	- Implemented new schedule for the Project based on Mid-term progress
	- Prepared a mid-term document in MarkDown for Mid-Term presentation
	- Debugging and fixing the waypoint system
	

##Features yet to be implemented:

#Week 9:
	-	Recording best run times in a save file
	- 	Setup waypoint system in the RaceTrack class
	-	Start working on RaceCar Audio

#Week 10:
	-	Implement RaceCar audio completely
		-	Setup 3D sound for car
		-	Tie in gear ratios to the vehicle audio

#Week 11:
	-	Implement a simple UI HUD
		-	Implement the track time display
		-	Implement lap tracker display 
		-	Implement tier 1 speedometer

#Week 12:
	-	Make a new race track
		-	Scale mode kit to be larger
		-	Setup new collision models
		- 	Implement new mesh file for track
	-	Create a racktrack class to handle track specific information
		-	Handle all Track checkpoint placements here

#Week 13:
	-	Make Game menus
	-	Implement a ghost car
	-	Implement Async loading 

#Week 14:
	-	Bug Fixes and Juice

#Week 15:
	-	Finals

##INSIGHTS:

#What went well:
	-	Tackled challenges early on
	-	Eliminated risks on Track system early
	-	Eliminated risks and bugs that rose from multiplayer 
	-	Fixed performance issues without any major hitches

#What went wrong:
	-	Performance bug was neglected for a while
	-	could not get to the audio setup which is the next major risk
	-	Should have communicated better with stakeholder when working on my track system
	-	Car feels sluggish
	-	Profiler worked but clicks were broken, spending some time to fix it would be very helpful

#What I would do differently:
	-	Communicate better with stake holders in terms of clarity and task changes
	-	Should have setup FixedUpdate as the first thing in any physics related project
	-	Make better plans early on rather than having to reduce my efficiency by having to go back and edit them
	-	Implement a better car in terms of feel