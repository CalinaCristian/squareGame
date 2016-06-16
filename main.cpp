
#include "lab_blackbox.hpp"
#include "lab_camera.hpp"


//time
#include <ctime>


#define KEY_ESCAPE	27
#define JUMP_DURATION 100
#define OBJECTS_EVERY_LOOP 28
#define HEIGHT_JUMP 4.0f
#define RESET_AFTER_DROPING -15

class Laborator5
	: public lab::glut::WindowListener
{
	private:
		// The BlackBox hides some functionality that we'll learn in the next course
		lab::BlackBox BLACKBOX;

		//SCENE_LOOP_MAX generates the scene on 5 levels (it can be infinite because it's made so it can be a loop)
		int SCENE_LOOP_MAX = 20;
		//GRAVITY makes the object fall if it isn't on another object. (everytime i make the scene bigger I have to 
		//set the gravity to a higher value because it takes more to load all the objects I think).
		float GRAVITY = 0.002f;

		//check if third person or first person camera is on.
		bool thirdPerson;

		//those are used so the object can jump and stay in the air for a little while.
		float heightJump = 0;
		float jumpDuration = 0;
		bool jumpMaxHeight = false;

		//used for animations when transitioning left and right with 90 degreees.
		float lastLeftRotation = 0;
		float lastRightRotation = 0;
		//cheat to skip thorugh levels (for presenting the homework faster).
		float lastNXT = 0;

		bool shouldRotateLeft = false;
		bool shouldRotateRight = false;
		float rotateTill90Left = 0;
		float rotateTill90Right = 0;

		//for the first person camera , to change the front when going backward.
		char direction = 'd';

		//displayLevel1 is printing something in console to show that the game started.
		bool displayLevel1 = true;
		//shouldFall is true when we are not on an object.
		bool shouldFall = false;
		//canJump is used so that the object can't jump if it is falling.
		bool canJump = true;
		//stopJumpingFrom is used to see where the object is falling and cannot jump anymore
		//beforeJumpingFrom and afterJumpingFrom are used for finding out if the next object or previous
		//object is under the camera so it can still jump.
		glm::vec3 stopJumpingFrom;
		glm::vec3 beforeJumpingFrom;
		glm::vec3 afterJumpingFrom;

		//Maximum number of cubes drawn in the scene (except for the camera).
		int maxObjects = OBJECTS_EVERY_LOOP * (SCENE_LOOP_MAX / 20) + 1;
		//an array of vec3 to retain the positions of the object when we redraw it over and over.
		//so it can be used when moving.
		glm::vec3 positions[1000];
		//iterator for positions array.
		int iterator = 0;

		// Buffers used for holding state of keys
		// true - pressed
		// false - not pressed
		bool keyState[256];
		bool specialKeyState[256];

		// Objects
		lab::Mesh *obiect;
		lab::Mesh *cameraTarget;

		// Projection matrix
		bool isPerspectiveProjection;
		glm::mat4 projectionMatrix;
		glm::mat4 projectionMatrix2;

		float FoV;
		float zNear, zFar;
		float aspectRatio;
		float orthoLeft, orthoRight, orthoTop, orthoBottom;
	

		// Camera
		lab::Camera camera;
		lab::Camera camera2;

	public:
		Laborator5()
		{
			// Load Objects
			obiect = BLACKBOX.loadMesh("resurse/cube2.obj");
			cameraTarget = BLACKBOX.loadMesh("resurse/cube2.obj");
			obiect->setColor(0, 0, 0);
			// init camera at the begining of the scene.
			camera.set(glm::vec3(18, 2, 42), glm::vec3(18, 2, 0), glm::vec3(0, 1, 0));
			//rotate the camera so it can be on the same side as fps camera
			camera.rotateTPS_OY(90);
			camera2.set(glm::vec3(camera.getTargetPosition().x + 1, camera.getTargetPosition().y + 2, camera.getTargetPosition().z + 1),
				glm::vec3(camera.getTargetPosition().x + 1, camera.getTargetPosition().y + 2, 0), glm::vec3(0, 1, 0));
			// initializa all key states to not pressed
			memset(keyState, 0, 256);
			memset(specialKeyState, 0, 256);

			// Initialize default projection values
			zNear = 0.1f;
			zFar = 500.0f;
			FoV = 40.0f;
			aspectRatio = 800 / 600.0f;
			orthoLeft = -15;
			orthoRight = 30;
			orthoBottom = -20;
			orthoTop = 25;

			// value may be used for updating the projection when reshaping the window
			isPerspectiveProjection = true;

			projectionMatrix = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, 0.1f, 500.0f);
			projectionMatrix2 = glm::perspective(FoV, aspectRatio, zNear, zFar);
		}

		~Laborator5()
		{
			delete obiect;
		}

		// Function used to compute perspective projection matrix
		// use glm::perspective and member variables FoV, aspectRation, zNear and zFar
		void computePerspectiveProjection()
		{
			projectionMatrix = glm::perspective(FoV, aspectRatio, zNear, zFar);
		}

		void computeOrthograhicProjection()
		{
			projectionMatrix = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, 0.1f, 500.0f);
		}

		//---------------------------------------------------------------------
		// Loop Functions - function that are called every single frame

		// Called right before frame update callback (notifyDisplayFrame)
		void notifyBeginFrame() { };

		void drawObjects(){
			// Send view matrix to the GPU and send projection matrix to the GPU
			if (thirdPerson){
				BLACKBOX.setViewMatrix(camera.getViewMatrix());
				BLACKBOX.setProjectionMatrix(projectionMatrix);
				// Draw the Third Person Camera target
				glm::vec3 targetPosition = camera.getTargetPosition();
				glm::mat4 targetModelMatrix = glm::translate(glm::mat4(1.0f), targetPosition);
				targetModelMatrix = glm::scale(targetModelMatrix, glm::vec3(0.1f));
				BLACKBOX.setModelMatrix(targetModelMatrix);
				BLACKBOX.drawMesh(cameraTarget);
			}
			//if second camera
			else {
				BLACKBOX.setViewMatrix(camera2.getViewMatrix());
				BLACKBOX.setProjectionMatrix(projectionMatrix2);
			}

			// ----------------------------------
			// Draw the object many times
			obiect->setColor(0.1, 0.1, 0.1);
			iterator = 0;

			//SCENE_LOOP_MAX makes it so the scene is higher when it's value is higher.
			for (int k = 0; k < SCENE_LOOP_MAX; k += 20){
				for (int j = 1; j < 10; j++){
					//draw the first line of 10 objects and save their positions.
					glm::mat4 objectModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f * j, 0 + k, 2.0f));

					positions[iterator++] = glm::vec3(2.0f * j, 0 + k, 2.0f);

					objectModelMatrix = glm::scale(objectModelMatrix, glm::vec3(0.1f));
					BLACKBOX.setModelMatrix(objectModelMatrix);
					BLACKBOX.drawMesh(obiect);
				}
				for (int j = 1; j < 6; j++){
					//draw the first 6 stairs 
					glm::mat4 objectModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f * j + k, (2.0f * j) + 2));

					positions[iterator++] = glm::vec3(2.0f, 2.0f * j + k, (2.0f * j) + 2);

					objectModelMatrix = glm::scale(objectModelMatrix, glm::vec3(0.1f));
					BLACKBOX.setModelMatrix(objectModelMatrix);
					BLACKBOX.drawMesh(obiect);
				}
				for (int j = 1; j < 10; j++){
					//draw the 2nd line of 10 objects.
					glm::mat4 objectModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f * j, 10 + k, (2.0f) + 10));

					positions[iterator++] = glm::vec3(2.0f * j, 10 + k, 2.0f + 10);

					objectModelMatrix = glm::scale(objectModelMatrix, glm::vec3(0.1f));
					BLACKBOX.setModelMatrix(objectModelMatrix);
					BLACKBOX.drawMesh(obiect);
				}
				for (int j = 1; j < 6; j++){
					//draw the 2nd 6 stairs.
					glm::mat4 objectModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f + 16, (2.0f * j) + 10 + k, (-2.0f * j) + 12));

					positions[iterator++] = glm::vec3(2.0f + 16, (2.0f * j) + 10 + k, (-2.0f * j) + 12);

					objectModelMatrix = glm::scale(objectModelMatrix, glm::vec3(0.1f));
					BLACKBOX.setModelMatrix(objectModelMatrix);
					BLACKBOX.drawMesh(obiect);
				}
			}
			//draw the green object.
			glm::mat4 objectModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f + 16, 0 + SCENE_LOOP_MAX, 2.0f - 2));

			positions[iterator++] = glm::vec3(2.0f + 16, 0 + SCENE_LOOP_MAX, 2.0f - 2);

			objectModelMatrix = glm::scale(objectModelMatrix, glm::vec3(0.1f));
			BLACKBOX.setModelMatrix(objectModelMatrix);
			//set it's color to green.
			obiect->setColor(0, 255, 0);
			BLACKBOX.drawMesh(obiect);
		}

		//function to do the gravitation.
		void gravitation(){
			shouldFall = true;
			for (int i = 0; i < maxObjects; i++) {
				//if we are on any of the objects drawn , we don't fall. We also save the previous, current and next object.
				if ((glm::abs(camera.getTargetPosition().x - positions[i].x) < 2.0f) &&
					( ( (camera.getTargetPosition().y - positions[i].y) < 2.0f) && ((camera.getTargetPosition().y - positions[i].y) > 0) ) &&
					(glm::abs(camera.getTargetPosition().z - positions[i].z) < 2.0f)){
					stopJumpingFrom = positions[i];
					if (i > 0){
						beforeJumpingFrom = positions[i - 1];
					}
					else {
						beforeJumpingFrom = positions[i];
					}
					if (i < maxObjects - 1){
						afterJumpingFrom = positions[i+1];
					}
					else {
						afterJumpingFrom = positions[i];
					}
					shouldFall = false;
				}
			}
			//if we are not on any object
			if (shouldFall){
				//if we fall from the object that we were on or the next or previous one, we can't jump anymore
				if (!((glm::abs(camera.getTargetPosition().x - stopJumpingFrom.x) < 2.0f) && 
					(glm::abs(camera.getTargetPosition().z -stopJumpingFrom.z) < 2.0f)) &&
					!((glm::abs(camera.getTargetPosition().x - beforeJumpingFrom.x) < 2.0f) &&
					(glm::abs(camera.getTargetPosition().z - beforeJumpingFrom.z) < 2.0f)) &&
					!((glm::abs(camera.getTargetPosition().x - afterJumpingFrom.x) < 2.0f) &&
					(glm::abs(camera.getTargetPosition().z - afterJumpingFrom.z) < 2.0f)) &&
					(camera.getTargetPosition().y < stopJumpingFrom.y + 4)){
					canJump = false;
				}
				//if we're at a certain level of negative height , we reset the level.
				if (camera.getTargetPosition().y < RESET_AFTER_DROPING){
					canJump = true;
					camera.set(glm::vec3(18, 2, 42), glm::vec3(18, 2, 0), glm::vec3(0, 1, 0));
					direction = 'd';
					camera.rotateTPS_OY(90);
					camera2.set(glm::vec3(camera.getTargetPosition().x + 1, camera.getTargetPosition().y + 2, camera.getTargetPosition().z + 1),
						glm::vec3(camera.getTargetPosition().x + 1, camera.getTargetPosition().y + 2, 0), glm::vec3(0, 1, 0));
				}
				//else , we just fall more.
				else {
					camera.translateUpword(-GRAVITY);
					camera2.translateUpword(-GRAVITY);
				}
			}
			else {
				//if we should not fall , we can jump.
				canJump = true;
			}
		}

		// Called every frame before we draw
		// Because glut sends only 1 key event every single frame, pressing more than 1 key will have no effect
		// If we treat the input using the 2 special buffers where we hold key states we can treat more than 1
		// key pressed at the same time. Also by not relling on glut to send the continuous pressing event
		// the moving motion will be more smooth because every single frame we apply the changes is contrast to
		// the event based method provided by Glut
		void treatInput()
		{
			//displaying the begining of the game.
			if (displayLevel1){
				displayLevel1 = false;
				printf("\n\n\n====================STARTING GAME!====================\n\n\n");
				printf("\n\n=======================LEVEL 1!=======================\n");
			}
			// Compute frameRate independent speed for moving the camera
			// We set the speed based on the frameTime between the last 2 frames
			// Speed will remain constant no matter how many frames/second is the GPU cappable to render
			float frameTime = BLACKBOX.getFrameTimeSeconds();
			float moveSpeed = frameTime * 10;
			float rotateSpeed = frameTime * 40;

			float rotateSpeedOX = frameTime * 40;
			float rotateSpeedOY = frameTime * 40;
			float rotateSpeedOZ = frameTime * 40;
			//secret key combination for showing all the levels without palying them.
			if (keyState['n'] && keyState['x'] && keyState['t']){
				if (glutGet(GLUT_ELAPSED_TIME) - lastNXT >= 1000){
					lastNXT = glutGet(GLUT_ELAPSED_TIME);
					switch (SCENE_LOOP_MAX){
					case 20:
						GRAVITY = 0.003f;
						SCENE_LOOP_MAX = 40;
						printf("\n=====================SKIPPING...======================\n");
						printf("\n=======================LEVEL 2!=======================\n");
						break;
					case 40:
						SCENE_LOOP_MAX = 60;
						GRAVITY = 0.004f;
						printf("\n=====================SKIPPING...======================\n");
						printf("\n=======================LEVEL 3!=======================\n");
						break;
					case 60:
						SCENE_LOOP_MAX = 80;
						GRAVITY = 0.005f;
						printf("\n=====================SKIPPING...======================\n");
						printf("\n=======================LEVEL 4!=======================\n");
						break;
					case 80:
						SCENE_LOOP_MAX = 100;
						GRAVITY = 0.006f;
						printf("\n=====================SKIPPING...======================\n");
						printf("\n=======================LEVEL 5!=======================\n");
						break;
					case 100:
						SCENE_LOOP_MAX = 20;
						GRAVITY = 0.002f;
						printf("\n\n========================CHEATER!========================\n\n");
						printf("======================RESTARTING...======================");
						displayLevel1 = true;
						break;
					}
					maxObjects = OBJECTS_EVERY_LOOP * (SCENE_LOOP_MAX / 20) + 1;
					camera.set(glm::vec3(18, 2, 42), glm::vec3(18, 2, 0), glm::vec3(0, 1, 0));
					direction = 'd';
					camera.rotateTPS_OY(90);
					camera2.set(glm::vec3(camera.getTargetPosition().x + 1, camera.getTargetPosition().y + 2, camera.getTargetPosition().z + 1),
						glm::vec3(camera.getTargetPosition().x + 1, camera.getTargetPosition().y + 2, 0), glm::vec3(0, 1, 0));
				}
			}
			if ((keyState['a'])) {
				if (direction == 'd'){
					camera2.rotateFPS_OY(180);
				}
				//we do the move
				camera.translateRight(-moveSpeed);
				camera2.translateForward(moveSpeed);
			
				//if we are coliding with an object , we cancel the move.
				for (int i = 0; i < maxObjects; i++){
					if  (( glm::abs(camera.getTargetPosition().x - positions[i].x) < 2.0f ) &&
						(glm::abs(camera.getTargetPosition().y - positions[i].y) < 1.9f) && 
						(glm::abs(camera.getTargetPosition().z - positions[i].z) < 2.0f)) {

						camera.translateRight(moveSpeed);
						
						camera2.translateForward(-moveSpeed);
					}
				}
				direction = 'a';
				//if we hit the last object (the green one) , we change the level / finish and restart from the first.
				if ((glm::abs(camera.getTargetPosition().x - positions[maxObjects - 1].x) < 2.0f) &&
					(((camera.getTargetPosition().y - positions[maxObjects - 1].y) < 2.0f) && ((camera.getTargetPosition().y - positions[maxObjects - 1].y) > 0)) &&
					(glm::abs(camera.getTargetPosition().z - positions[maxObjects - 1].z) < 2.0f)){

					switch (SCENE_LOOP_MAX){
					case 20:
						GRAVITY = 0.003f;
						SCENE_LOOP_MAX = 40;
						printf("\n\n=======================LEVEL 2!=======================\n");
						break;
					case 40:
						SCENE_LOOP_MAX = 60;
						GRAVITY = 0.004f;
						printf("\n=======================LEVEL 3!=======================\n");
						break;
					case 60:
						SCENE_LOOP_MAX = 80;
						GRAVITY = 0.005f;
						printf("\n=======================LEVEL 4!=======================\n");
						break;
					case 80:
						SCENE_LOOP_MAX = 100;
						GRAVITY = 0.006f;
						printf("\n=======================LEVEL 5!=======================\n");
						break;
					case 100:
						SCENE_LOOP_MAX = 20;
						GRAVITY = 0.002f;
						printf("\n\n===============CONGRATIULATIONS! YOU WON!===============\n\n");
						printf("======================RESTARTING...======================");
						displayLevel1 = true;
						break;
					}
					//recalculate the maxObjects and reset the camera position to the begining of the scene.
					maxObjects = OBJECTS_EVERY_LOOP * (SCENE_LOOP_MAX / 20) + 1;
					camera.set(glm::vec3(18, 2, 42), glm::vec3(18, 2, 0), glm::vec3(0, 1, 0));
					camera.rotateTPS_OY(90);
					direction = 'd';
					camera2.set(glm::vec3(camera.getTargetPosition().x + 1, camera.getTargetPosition().y + 2, camera.getTargetPosition().z + 1),
						glm::vec3(camera.getTargetPosition().x + 1, camera.getTargetPosition().y + 2, 0), glm::vec3(0, 1, 0));
				}
			}
			//same for d.
			if ((keyState['d'])) { 
				if (direction == 'a'){
					camera2.rotateFPS_OY(180);
				}
				camera.translateRight(moveSpeed);
				camera2.translateForward(moveSpeed);
				for (int i = 0; i < maxObjects; i++){
					if ((glm::abs(camera.getTargetPosition().x - positions[i].x) < 2.0f) &&
						(glm::abs(camera.getTargetPosition().y - positions[i].y) < 1.9f) &&
						(glm::abs(camera.getTargetPosition().z - positions[i].z) < 2.0f)) {

						camera.translateRight(-moveSpeed);

						camera2.translateForward(-moveSpeed);
					}
				}
				direction = 'd';
				if ((glm::abs(camera.getTargetPosition().x - positions[maxObjects - 1].x) < 2.0f) &&
					(((camera.getTargetPosition().y - positions[maxObjects - 1].y) < 2.0f) && ((camera.getTargetPosition().y - positions[maxObjects - 1].y) > 0)) &&
					(glm::abs(camera.getTargetPosition().z - positions[maxObjects - 1].z) < 2.0f)){

					switch (SCENE_LOOP_MAX){
					case 20:
						GRAVITY = 0.003f;
						SCENE_LOOP_MAX = 40;
						printf("\n\n=======================LEVEL 2!=======================\n");
						break;
					case 40:
						SCENE_LOOP_MAX = 60;
						GRAVITY = 0.004f;
						printf("\n=======================LEVEL 3!=======================\n");
						break;
					case 60:
						SCENE_LOOP_MAX = 80;
						GRAVITY = 0.005f;
						printf("\n=======================LEVEL 4!=======================\n");
						break;
					case 80:
						SCENE_LOOP_MAX = 100;
						GRAVITY = 0.006f;
						printf("\n=======================LEVEL 5!=======================\n");
						break;
					case 100:
						SCENE_LOOP_MAX = 20;
						GRAVITY = 0.002f;
						printf("\n\n===============CONGRATIULATIONS! YOU WON!===============\n\n");
						printf("======================RESTARTING...======================");
						displayLevel1 = true;
						break;
					}
					maxObjects = OBJECTS_EVERY_LOOP * (SCENE_LOOP_MAX / 20) + 1;
					camera.set(glm::vec3(18, 2, 42), glm::vec3(18, 2, 0), glm::vec3(0, 1, 0));
					direction = 'd';
					camera.rotateTPS_OY(90);
					camera2.set(glm::vec3(camera.getTargetPosition().x + 1, camera.getTargetPosition().y + 2, camera.getTargetPosition().z + 1),
						glm::vec3(camera.getTargetPosition().x + 1, camera.getTargetPosition().y + 2, 0), glm::vec3(0, 1, 0));
				}
			}
			//does the gravitation
			gravitation();

			if (specialKeyState[GLUT_KEY_LEFT]) { 
				//if we pressed the button after 500 miliseconds (so it doesn't do it too fast)
				//because one click counts as more , we set the boolean to true so it can rotate on release.
				if (glutGet(GLUT_ELAPSED_TIME) - lastLeftRotation >= 500){
					shouldRotateLeft = true;
					lastLeftRotation = glutGet(GLUT_ELAPSED_TIME);
				}
			}
			else if (!specialKeyState[GLUT_KEY_LEFT]){
				if (shouldRotateLeft && rotateTill90Left < 90){
					rotateTill90Left += 0.25f;
					camera.rotateTPS_OY(-0.25f);
					camera2.rotateFPS_OY(-0.25f);
				}
				else if (shouldRotateLeft){
					shouldRotateLeft = false;
					rotateTill90Left = 0;
				}
			}
			//same for right
			if (specialKeyState[GLUT_KEY_RIGHT]) { 
				if (glutGet(GLUT_ELAPSED_TIME) - lastRightRotation >= 500){
					shouldRotateRight = true;
					lastRightRotation = glutGet(GLUT_ELAPSED_TIME);
				}
			}
			else if (!specialKeyState[GLUT_KEY_RIGHT]){
				if (shouldRotateRight && rotateTill90Right < 90){
					rotateTill90Right += 0.25f;
					camera.rotateTPS_OY(0.25f);
					camera2.rotateFPS_OY(0.25f);
				}
				else if (shouldRotateRight){
					shouldRotateRight = false;
					rotateTill90Right = 0;
				}
			}
			//if we want to jump
			if (keyState[' ']) {
				//if we can jump and we haven't reached the maximum allowed jump (HEIGHTJUMP)
				bool fall = true;
				for (int i = 0; i < maxObjects; i++){
					if ((glm::abs(camera.getTargetPosition().x - positions[i].x) < 2.0f) &&
						(((camera.getTargetPosition().y - positions[i].y) < 4.0f) && ((camera.getTargetPosition().y - positions[i].y) > 0)) &&
						(glm::abs(camera.getTargetPosition().z - positions[i].z) < 2.0f)){
						
						fall = false;
					}
				}
				if (!fall) {
					if ((heightJump < HEIGHT_JUMP) && (!jumpMaxHeight) && (canJump)){
						//we do the jump faster than the gravitation so it doesn't fall faster than it jumps.
						jumpDuration = 0.1f;
						heightJump += moveSpeed;
						camera.translateUpword(moveSpeed);
						camera2.translateUpword(moveSpeed);
					}
					else if ((jumpDuration > 0) && (jumpDuration <= JUMP_DURATION)){
						//when we reached the maximum jump , we hold there for a while(the gravity pulls us back anyway).
						jumpMaxHeight = true;
						jumpDuration += 0.4f;
					}
					else {
						if (heightJump > 0){
							heightJump -= moveSpeed;
						}
						else {
							jumpDuration = 0;
							jumpMaxHeight = false;
						}
					}
				}
			}
		}

		// A key was pressed
		void notifyKeyPressed(unsigned char key_pressed, int mouse_x, int mouse_y)
		{
			keyState[key_pressed] = 1;
			if (key_pressed == KEY_ESCAPE)
			{
				lab::glut::close();
			}

			if (key_pressed == 'o')
			{
				computeOrthograhicProjection();
			}

			if (key_pressed == 'p')
			{
				computePerspectiveProjection();
			}

			// Reset Camera
			if (keyState['r'])
			{
				canJump = true;
				camera.set(glm::vec3(18, 2, 42), glm::vec3(18, 2, 0), glm::vec3(0, 1, 0));
				direction = 'd';
				camera.rotateTPS_OY(90);
				camera2.set(glm::vec3(camera.getTargetPosition().x + 1, camera.getTargetPosition().y + 2, camera.getTargetPosition().z + 1),
					glm::vec3(camera.getTargetPosition().x + 1, camera.getTargetPosition().y + 2, 0), glm::vec3(0, 1, 0));
			}

		}

		// When a key was released
		void notifyKeyReleased(unsigned char key_released, int mouse_x, int mouse_y)
		{
			keyState[key_released] = 0;
		}

		// Special key pressed like the navigation arrows or function keys F1, F2, ...
		void notifySpecialKeyPressed(int key_pressed, int mouse_x, int mouse_y)
		{
			specialKeyState[key_pressed] = 1;
			switch (key_pressed)
			{
			case GLUT_KEY_F1: {
				lab::glut::enterFullscreen();
				break;
			}

			case GLUT_KEY_F2: {
				lab::glut::exitFullscreen();
				break;
			}

			case GLUT_KEY_F5: {
				BLACKBOX.LoadShader();
				break;
			}

			default:
				break;
			}
		}

		// Called when a special key was released
		void notifySpecialKeyReleased(int key_released, int mouse_x, int mouse_y)
		{
			specialKeyState[key_released] = 0;
		}

		// Called every frame to draw
		void notifyDisplayFrame()
		{
			// Treat continuous input
			treatInput();

			// ----------------------------------
			// Set the viewport and view and projection matrices

			unsigned int width = lab::glut::getInitialWindowInformation().width;
			unsigned int height = lab::glut::getInitialWindowInformation().height;

			// Clear Color Buffer with the specified color
			glScissor(0, 0, width, height);
			glEnable(GL_SCISSOR_TEST);
			glClearColor(0.3, 0.123, 0.53, 0);
			glClear(GL_COLOR_BUFFER_BIT);
			BLACKBOX.notifyDisplay();

			glViewport(0, 0, width, height);

			thirdPerson = true;
			drawObjects();
			
			glViewport(0.7 * width, 0.7 * height, 0.3 * width, 0.3 * height);
			glScissor(0.7 * width, 0.7 * height, 0.3 * width, 0.3 * height);
			glEnable(GL_SCISSOR_TEST);
			glClearColor(0.53, 0.13, 0.63, 0);
			glClear(GL_COLOR_BUFFER_BIT);

			thirdPerson = false;
			drawObjects();

		}

		// Called when the frame ended
		void notifyEndFrame() { }

		//---------------------------------------------------------------------
		// Function called when the windows was resized
		void notifyReshape(int width, int height, int previos_width, int previous_height)
		{
			//blackbox needs to know
			BLACKBOX.notifyReshape(width, height);
			aspectRatio = (float)width / height;

		}

		//---------------------------------------------------------------------
		// Input function

		// Mouse drag, mouse button pressed 
		void notifyMouseDrag(int mouse_x, int mouse_y) { }

		// Mouse move without pressing any button
		void notifyMouseMove(int mouse_x, int mouse_y) { }

		// Mouse button click
		void notifyMouseClick(int button, int state, int mouse_x, int mouse_y) { }

		// Mouse scrolling
		void notifyMouseScroll(int wheel, int direction, int mouse_x, int mouse_y) { }
};

int main()
{
	// Initialize GLUT: window + input + OpenGL context
	lab::glut::WindowInfo window(std::string("EGC Laborator 5 - Camera and Projections"), 800, 600, 600, 100, true);
	lab::glut::ContextInfo context(3, 3, false);
	lab::glut::FramebufferInfo framebuffer(true, true, false, false);
	lab::glut::init(window, context, framebuffer);

	// Initialize GLEW + load OpenGL extensions 
	glewExperimental = true;
	glewInit();
	std::cout << "[GLEW] : initializare" << std::endl;

	// Create a new instance of Lab and listen for OpenGL callback
	// Must be created after GLEW because we need OpenGL extensions to be loaded

	Laborator5 *lab5 = new Laborator5();
	lab::glut::setListener(lab5);

	// Enter loop
	lab::glut::run();

	return 0;
}