# DFS-2 Project plan revision (04/07/2020)

Name : Pronay
Cohort: C28
Project: TAR (Time Attack Racer)

Notes about this document:

- Each task has been assigned estimated time, task desciption and a NewTask ID. Format followed is below:
	[TimeEstimate] <Task Desciption> (NewTask ID: ##)
- Under each week's "Proposed plan" is an "Hours worked" section to be updated with actual time spent on task and the date for when it was done.
- Each week has it's total hours planned listed along with space to list the total hours worked. 


## WEEK 12: GAME LOOP COMPLETION AND CAR TUNING

### Plan Proposed:


	-	[4]	Create a Simple Menu  (NewTask ID: 5)
		-	[2] Implement widget system from RTS to have "Play", "Tune Car", "Quit" functionality
		-	[1] Implement some statefull-ness (LOADING_STATE, MENU_STATE, GAME_STATE, END_STATE)
		-	[1] Create a duplicate menu for end of race to have "REPLAY", "RETURN TO MENU" functionality

	-	[4] Implement a car HUD  (NewTask ID: 6)
		-	[2] Implement a radial speedometer
		-	[1] Implement a lap tracker using each player's waypoint system
		-	[1] Display current times for all players

	-	[3] Implement Car Dev Tool (NewTask ID: 3)
		-	[2] Expose all car variables to ImGUI debug widget interface
		-	[1] Setup data and interface required to send vehicle info to PhysXSystem

	-	[3]	Tune the car(NewTask ID: 7)
		-	Find good values for the car

### Hours Worked:

	-	[ ] (04/00/2020) Create a tune system(NewTask ID: 4)
	-	[ ] (04/00/2020) Create a Simple Menu  (NewTask ID: 5)
	-	[ ] (04/00/2020) Implement a car HUD  (NewTask ID: 6)
	-	[ ] (04/00/2020) Tune the car (NewTask ID: 7)

	-	[5] Simple Menu
	-	[4] PhysX bug on car (Un-solved)
	-	[4] Quaternion bugs on reset
	-	[9] Car Tool 
	- 	[3] Car Tuning

Total: 25 hours

Total Hours Planned: 14 Hours
Total Hours Worked: 25 Hours

## WEEK 13: TRACK REFINEMENT AND PLAYER CONVEYANCE

### Plan Proposed:

	-	[6]	Fix track and car scale(NewTask ID: 9)
		-	[4]	Resize track to be bigger so lane is a good size for 2 cars
		-	[2]	Test with 2 cars and make refinements

	-	[3]	Conveyance for player(NewTask ID: 10)
		-	[1]	Add different vertex colors to change cars 

### Hours Worked:

	-	[ ] (04/00/2020) Add more meshes to scene(NewTask ID: 8)
	-	[ ] (04/00/2020) Fix track and car scale(NewTask ID: 9)
	-	[ ] (04/00/2020) Conveyance for player(NewTask ID: 10)

Total Hours Planned: 12 Hours
Total Hours Worked: X Hours

## WEEK 14: POLISH ARTIFACT

### Plan Proposed:

	-	[3]	Revisit audio(NewTask ID: 11)
		-	[1]	Add menu audio
		-	[1]	Implement audio for race start, race end
		-	[1]	Refine vehicle audio 

	-	[6]	Testing and Debugging all features of the game (NewTask ID: 14)

### Hours Worked:

	-	[ ] (04/00/2020) Revisit audio(NewTask ID: 11)
	-	[ ] (04/00/2020) Refine art in the game (Changing any textures/models if required) (NewTask ID: 12)
	-	[ ] (04/00/2020) Implement camera shake on car when bumping into other players (NewTask ID: 13)
	-	[ ] (04/00/2020) Testing and Debugging all features of the game (NewTask ID: 14)

Total Hours Planned: 12 Hours
Total Hours Worked: X Hours
