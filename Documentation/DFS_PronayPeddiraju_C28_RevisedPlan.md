#DFS-2 Project Planning

Name : Pronay
Cohort: C28
Project: TAR (Time Attack Racer)

Learnings from previous plans:

	-	Need a better way to organize per week plans
	-	Need to handle hours planned vs hours worked better
	-	Need some form of scrum when developing just to maintain sanity and ensure the correct tasks are being tackled

Solution to the planning problem
	
##Task Management:
	-	Keep all tasks listed in Github board for speed and ease of use
	-	Keep To-Do, In-Progress, Done and Cancelled Task columns for general work flow
	- 	Keep a Debug Features column for when things that are implemented need to be re-worked

##Weekly plan management:
	-	Since Github can't do everything, maintain a .md file that has a weekly breakdown
	-	Account for hours on weekly breakdown for both planned and worked hours
	-	Have a completion section where I can add the date of completion for task and number of hours used

#DFS Mid Term Plan:

NOTE: Each task has it's hours planned in [ ] before the task header. Each task header also has a (Task ID: #) where each task is assigned an ID, these IDs correspond to matching tasks on Github project boards. Weekly breakdowns below:

	-	[PlannedTime]	Task Header 	(Task ID : ##)

Find the TAR Project board here: https://github.com/pronaypeddiraju/TimeAttackRacerBoards/projects/1?fullscreen=true

NOTE: The entire plan has been listed in this file along with hourly breakdowns and space to update actual hours worked for each task. The GitHub board is just for my personal use and contains no informaiton that this file doesn't contain. All Task IDs mentioned in this file correspond to tasks in the GitHub project board linked above.

##WEEK 9: VEHICLE AUDIO IMPLEMENTATION

###Plan Proposed:

	-	[3] Research on Vehicle Audio (Task ID: 1)
		-	How is Audio generally implemented for Racing Games
		-	Types of Audio files I need to use
		-	Any systemic changes required in AudioSystem?

	-	[3]	Vehicle Audio Setup (Task ID: 2)
		-	[2] Design system for Audio playback on Car class
			-	Where and when to play 3D spatial audio 
			-	Implement m_car.GetCarController().GetGearRatiosForAudio()
			-	Implement m_car.PlayAudioForGear( gearRatios, currentGear);

	-	[3] AudioSystem Implementation Changes (Task ID: 3)
		-	Account for any changes required by Audio System class to handle playback changes from general 

###On Plan Completion:

Enter the actual house used here as well as the date you worked on this task in the only correct date format (DD/MM/YYYY). Fight me on date format.

	-	[4] (00/03/2020) Research on Vehicle Audio (Task ID: 1)
	-	[6] (00/03/2020) Vehicle Audio Setup (Task ID: 2)
	-	[2] (00/03/2020) AudioSystem Implementation Changes (Task ID: 3)

Additional Tasks Performed:

	-	[3]	Debugging and Bug fixing on SplitScreen System

Total Hours Planned: 9 Hours
Total Hours Worked: X Hours

##WEEK 10: WAYPOINT SYSTEM AND VEHICLE UPDATES

###Plan Proposed:

	-	[3]	Audio Debug and Integrate (Task ID: 4)

	-	[3] Waypoint Time Tracking (Task ID: 5)
		-	[1.5] Implement fastest time tracking on waypoint system for all 4P
		-	[1.5] Write functions for save and load using xml

	-	[3]	Update Vehicle Creation Logic (Task ID: 6)
		-	[1] Update vehicle mesh dimensions to be more accurate for car convex mesh
		-	[1] Expose engine, differential and transmission variables
		-	[1] Expose sprung mass settings for wheels

###On Plan Completion:

Enter the actual house used here as well as the date you worked on this task in the only correct date format (DD/MM/YYYY). Fight me on date format.

	-	[ ] (00/03/2020) Audio Debug and Integration (Task ID: 4)
	-	[2] (00/03/2020) Waypoint Time Tracking (Task ID: 5)
	-	[ ] (00/03/2020) Update Vehicle Creation Logic (Task ID: 6)

Other Tasks Performed:

	-	[5]	Fixing issues with Debug build cause by using static variables
	-	[1]	Fixed issues with Audio for other players

Total Hours Planned: 9 Hours
Total Hours Worked: X Hours

##WEEK 11: CAR TUNING AND ASYNC RESOURCE LOAD

###Plan Proposed:

	-	[3] Implement Car Tuning Tool (Task ID: 7)
		-	[1] Expose all car variables to ImGUI debug widget interface
		-	[2] Find good defaults for vehicle settings

	-	[3]	Async Resource Loader (Task ID: 8)
		-	[1] Implement async resource loading thread
		-	[1] Implement load logic for all game meshes
		-	[1] Debug and Integration

	-	[3] Make Game menus (Task ID: 9)
		-	[1] Implement a UI main menu 
		-	[2] Implement navigation and selection logic for controller input

###On Plan Completion:

Enter the actual house used here as well as the date you worked on this task in the only correct date format (DD/MM/YYYY). Fight me on date format.

	-	[ ] (00/03/2020) Implement Car Tuning Tool (Task ID: 7)
	-	[ ] (00/03/2020) Async Resource Loader (Task ID: 8)
	-	[ ] (00/03/2020) Make Game Menus (Task ID: 9)

Total Hours Planned: 9 Hours
Total Hours Worked: X Hours

##WEEK 12: HUD IMPLEMENTATION AND NEW RACETRACK

###Plan Proposed:

	-	[3]	Create Vehicle HUD  (Task ID: 10)
		-	[1] Create a speedometer widget to use car momentum
		-	[1] Display lap information and best times
		-	[1] Render HUD with both UI widgets

	-	[3] Test and Refine HUD  (Task ID: 11)
		-	[1] Adjust widget placement
		-	[1] Debug speedometer using debug values on screen
		-	[1] Debug lap system display for all 4P recorded times

	-	[3]	Model New Race Track (Task ID: 12)
		-	[1]	Scale all mod kit pieces to be larger
		-	[2]	Implement new tack design

	-	[3] Model Race Track Collisions and meta-data (Task ID: 13)
		-	[2] Model collision meshes to be used for PhysX
		-	[1] Setup .mesh file to load all the collision mesh information


###On Plan Completion:

Enter the actual house used here as well as the date you worked on this task in the only correct date format (DD/MM/YYYY). Fight me on date format.

	-	[ ] (00/03/2020) Create Vehicle HUD (Task ID: 10)
	-	[ ] (00/03/2020) Test and Refine HUD (Task ID: 11)
	-	[ ] (00/04/2020) Model New Race Track (Task ID: 12)
	-	[ ] (00/03/2020) Model Race Track Collisions and meta-data (Task ID: 13)

Total Hours Planned: 12 Hours
Total Hours Worked: X Hours

##WEEK 13: 

###Plan Proposed: GHOST CAR AND DATA_DRIVEN CHECKPOINTS

	-	[3]	Implement Ghost Car Section 1 (Task ID: 14)
		-	[1] Track car positions through run 
		-	[1] Save off ghost car positions in xml 
		-	[1] Load ghost car positions from xml

	-	[3] Implement Ghost Car Section 2 (Task ID: 15)
		-	[1] Track wheel rotation for vehicle
		-	[2] Debug and Integration

	-	[3] Checkpoint System Data Driving (Task ID: 16)
		-	Implement fastest time tracking on waypoint system for all 4P
		-	Write functions for save and load using xml

###On Plan Completion:

Enter the actual house used here as well as the date you worked on this task in the only correct date format (DD/MM/YYYY). Fight me on date format.

	-	[ ] (00/03/2020) Implement Ghost Car Section 1 (Task ID: 14)
	-	[ ] (00/03/2020) Implement Ghost Car Section 2 (Task ID: 15)
	-	[ ] (00/03/2020) Checkpoint System Data Driving (Task ID: 16)

Total Hours Planned: 9 Hours
Total Hours Worked: X Hours

##WEEK 14: 

###Plan Proposed: CONVEYANCE AND JUICE

	-	[3]	Add Player Vehicle Conveyance (Task ID: 17)
		-	[1] Setup different colors for different player cars
		-	[1] Setup some on-screen P1,2,3,4 identification info
		-	[1] Test and Debug

	-	[3]	Implement Controller Join Screen (Task ID: 18)
		-	[1] Check for input before assigning contoller to cars
		-	[1] Show player IDs for connected controllers
		-	[1] Add join screen to OnClick play button

	-	[3]	Tier 1 Juice (Task ID: 19)
		-	[2] Implement camera shake on hit cars
		-	[1] Implement a skybox tier 1

	-	[3]	Tier 2 Juice (Task ID: 20)
		-	[2] Implement sky box tier 2
		-	[1] Refine UI

###On Plan Completion:

Enter the actual house used here as well as the date you worked on this task in the only correct date format (DD/MM/YYYY). Fight me on date format.

	-	[ ] (00/03/2020) Add Player Vehicle Conveyance (Task ID: 17)
	-	[ ] (00/03/2020) Implement Controller Join Screen (Task ID: 18)
	-	[ ] (00/03/2020) Tier 1 Juice (Task ID: 19)
	-	[ ] (00/03/2020) Tier 2 Juice (Task ID: 20)


Total Hours Planned: 12 Hours
Total Hours Worked: X Hours

##WEEK 15: 

###Plan Proposed: BUG FIXING AND WISHLIST TASKS

	-	[3] Bug Fixing (Task ID: 21)

Wishlist Tasks:

	-	[5]	Model 2nd Track (Wishlist ID: 1)
		-	[2] Model a new track
		-	[2] Model collisions
		-	[1] Setup .mesh file

	-	[6] Add a second car (Wishlist ID: 2)
		-	[1] Acquire car off the internet 
		-	[2] Setup mesh file for car
		-	[2] Tune car values

###On Plan Completion:

Enter the actual house used here as well as the date you worked on this task in the only correct date format (DD/MM/YYYY). Fight me on date format.

	-	[ ] (00/03/2020) Bug Fixing (Task ID: 21)
	-	[ ] (00/03/2020) Model 2nd Track (Wishlist ID: 1)
	-	[ ] (00/03/2020) Add a second car (Wishlist ID: 2)

Total Hours Planned: 14 Hours
Total Hours Worked: X Hours