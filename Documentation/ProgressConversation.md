#DFS 2 Progress Conversation 

Date: 03-April-2020

Current Plan:

##WEEK 10: WAYPOINT SYSTEM AND VEHICLE UPDATES (This week)

###Plan Proposed:

	-	[3]	Audio Debug and Integrate (Task ID: 4)

	-	[3] Waypoint Time Tracking (Task ID: 5)
		-	[1.5] Implement fastest time tracking on waypoint system for all 4P
		-	[1.5] Write functions for save and load using xml

	-	[3]	Update Vehicle Creation Logic (Task ID: 6)
		-	[1] Update vehicle mesh dimensions to be more accurate for car convex mesh
		-	[1] Expose engine, differential and transmission variables
		-	[1] Expose sprung mass settings for wheels

###Work Done:

	-	[1] (04/01/2020) Audio Debug and Integration (Task ID: 4)
	-	[2] (04/02/2020) Waypoint Time Tracking (Task ID: 5)
			-	Only implemented fastest time tracking for best lap and entire race for all 4P
			-	Need to still do XML save and load task

	-	[ ] (00/03/2020) Update Vehicle Creation Logic (Task ID: 6)
			- Couldn't get to this

Other Tasks Performed:

	-	[3]	(04/02/2020) Fixed issues with Audio for players apart from P1
	-	[2]	(04/02/2020) Fixing issues with Debug build cause by using static variables
	-	[4]	(04/03/2020) Fixing issues with Renderer and D3D leaks 
			-	Identified source of leak
			-	Performed numerous test scenarios to determine cause of leak
			-	Found issues with bind/unbind of GPU buffer but couldn't understand how exactly to fix it
			-	Scheduled discussion with Forseth over the weekend to discuss my problem and show him what I'm doing

Total Hours Planned: 9 Hours
Total Hours Worked: 12 Hours

##WEEK 11: CAR TUNING AND ASYNC RESOURCE LOAD (Next Week)

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

#What I propose as a new Plan

Re-shuffle some of the tasks between Week 12 and 11. Finish up what I know can be done easily and boost morale to tackle framework tasks including car tool;

##WEEK 11: ASYNC RESOURCE LOAD, HUD IMPLEMENTATION (Next Week)

###Plan Proposed:

	-	[3]	Async Resource Loader (Task ID: 8)
		-	[1] Implement async resource loading thread
		-	[1] Implement load logic for all game meshes
		-	[1] Debug and Integration

	-	[3] Make Game menus (Task ID: 9)
		-	[1] Implement a UI main menu 
		-	[2] Implement navigation and selection logic for controller input

	-	[3]	Create Vehicle HUD  (Task ID: 10)
		-	[1] Create a speedometer widget to use car momentum
		-	[1] Display lap information and best times
		-	[1] Render HUD with both UI widgets

	-	[3] Test and Refine HUD  (Task ID: 11)
		-	[1] Adjust widget placement
		-	[1] Debug speedometer using debug values on screen
		-	[1] Debug lap system display for all 4P recorded times

Total Hours Planned: 12 Hours
Total Hours Worked: X Hours

##WEEK 12: CAR TOOL AND NEW RACETRACK

###Plan Proposed:

	-	[3] Implement Car Tuning Tool (Task ID: 7)
		-	[1] Expose all car variables to ImGUI debug widget interface
		-	[2] Find good defaults for vehicle settings

	-	[3]	Model New Race Track (Task ID: 12)
		-	[1]	Scale all mod kit pieces to be larger
		-	[2]	Implement new tack design

	-	[3] Model Race Track Collisions and meta-data (Task ID: 13)
		-	[2] Model collision meshes to be used for PhysX
		-	[1] Setup .mesh file to load all the collision mesh information

Total Hours Planned: 9 Hours
Total Hours Worked: X Hours