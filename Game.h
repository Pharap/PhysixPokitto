/*
   Copyright (C) 2018 Pharap (@Pharap)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

#include "Physics.h"

#include <Pokitto.h>

class Game
{

public:
	// Simulates friction
	// Not actually how a real coefficient of friction works
	static constexpr Number CoefficientOfFriction = 0.95;

	// Simulates gravity
	// Earth's gravitational pull is 9.8 m/sb_type
	// But that's far too powerful for the tiny screen
	// So I picked something small
	static constexpr Number CoefficientOfGravity = 0.5;

	// Simulates bounciness
	// Again, not quite like the real deal
	static constexpr Number CoefficientOfRestitution = 0.3;

	// Prevents never-ending bounciness
	static constexpr Number RestitutionThreshold = Number::Epsilon * 16;

	// Amount of force the player exerts
	static constexpr Number InputForce = 0.25;

private:
	RigidBody objects[8];

	// playerObject always points to objects[0]
	// The two can be considered interchangeable
	RigidBody & playerObject = objects[0];

	bool gravityEnabled = false;
	Vector2 gravitationalForce = Vector2(0, CoefficientOfGravity);

	bool statRenderingEnabled = true;

public:

	void randomiseObjects(void)
	{
		using namespace Pokitto;

		for(uint8_t i = 0; i < arrayLength(objects); ++i)
		{
			RigidBody & object = objects[i];

			object.position = Point2(Number(rand() % Display::getWidth()), Number(rand() % Display::getHeight()));
			if(gravityEnabled)
				// If gravity enabled, only affect y
				object.velocity.y += Number(-8 + rand() % 16, rand() % (1 << Number::FractionSize));
			else
				// If gravity not enabled, affect both
				object.velocity += Vector2(Number(-8 + rand() % 16, rand() % (1 << Number::FractionSize)), Number(-8 + rand() % 16, rand() % (1 << Number::FractionSize)));
		}
	}

	void main(void)
	{
		using namespace Pokitto;

		Core::begin();
		this->setup();
		while (Core::isRunning())
			if (Core::update())
				this->loop();
	}

	void setup(void)
	{
		using namespace Pokitto;

		randomiseObjects();

		playerObject.position = Point2(Number(Display::getWidth() / 2), Number(Display::getHeight() / 2));
		playerObject.velocity = Vector2(0, 0);
	}

	void loop(void)
	{
		using namespace Pokitto;

		//Buttons::pollButtons();

		updateInput();
		simulatePhysics();

		Display::setColor(0);
		//Display::clear();

		Display::setColor(1);
		renderObjects();

		if(statRenderingEnabled)
			renderDisplay();

		//Display::update();
	}

	void renderObjects(void)
	{
		using namespace Pokitto;

		for(uint8_t i = 0; i < arrayLength(objects); ++i)
		{
			RigidBody & object = objects[i];
			if(i > 0)
				Display::fillRect(static_cast<int8_t>(object.getX()), static_cast<int8_t>(object.getY()), 8, 8);
			else
				Display::drawRect(static_cast<int8_t>(object.getX()), static_cast<int8_t>(object.getY()), 8, 8);
		}
	}

	int debugCounter = 0;

	void renderDisplay(void)
	{
		using namespace Pokitto;

		Display::println("Gravity");
		Display::println(gravityEnabled ? "ON" : "OFF");
		Display::println(gravitationalForce.y < 0 ? "UP" : "DOWN");

		Display::print("G: ");
		Display::println(static_cast<float>(CoefficientOfGravity));
		Display::print("F: ");
		Display::println(static_cast<float>(CoefficientOfFriction));
		Display::print("R: ");
		Display::println(static_cast<float>(CoefficientOfRestitution));

		++debugCounter;
		Display::print("D: ");
		Display::println(debugCounter);
	}

	void updateInput(void)
	{
		using namespace Pokitto;

		// Input tools for playing around
		// Buttons::pressed(BTN_B)
		if(Buttons::repeat(BTN_B, 1))
		{
			// A - shake up the other objects by applying random force
			if(Buttons::held(BTN_A, 1))
				randomiseObjects();

			// Down - toggle gravity on/off
			if(Buttons::held(BTN_DOWN, 1))
				gravityEnabled = !gravityEnabled;

			// Up - invert gravity
			if(Buttons::held(BTN_UP, 1))
				gravitationalForce = -gravitationalForce;

			// Left - toggle statRenderingEnabled on/off
			if(Buttons::held(BTN_LEFT, 1))
				statRenderingEnabled = !statRenderingEnabled;
		}
		// Input for normal object control
		else
		{
			Vector2 playerForce = Vector2(0, 0);

			if(Buttons::held(BTN_LEFT, 1))
				playerForce.x += -InputForce;

			if(Buttons::held(BTN_RIGHT, 1))
				playerForce.x += InputForce;

			if(Buttons::held(BTN_UP, 1))
				playerForce.y += -InputForce;

			if(Buttons::held(BTN_DOWN, 1))
				playerForce.y += InputForce;

			// The player's input can be thought of as a force
			// to be enacted on the object that the player is controlling
			playerObject.velocity += playerForce;

			// Emergency stop
			if(Buttons::held(BTN_A, 1))
				playerObject.velocity = Vector2(0, 0);
		}
	}

	void simulatePhysics(void)
	{
		using namespace Pokitto;

		// Update objects
		for(uint8_t i = 0; i < arrayLength(objects); ++i)
		{
			// object refers to the given item in the array
			RigidBody & object = objects[i];

			// First, simulate gravity
			if(gravityEnabled)
				object.velocity += gravitationalForce;

			// Then, simulate friction
			if(gravityEnabled)
				// If gravity is enabled, just simulate horizontal friction
				object.velocity.x *= CoefficientOfFriction;
			else
				// If gravity isn't enabled, simulate top-down friction
				object.velocity *= CoefficientOfFriction;

			// Then, keep the objects onscreen
			// (A sort of cheaty way of keeping the objects onscreen)

			// They're literally bouncing off the walls :P
			if(object.position.x < 0)
			{
				object.position.x = 0;
				object.velocity.x = -object.velocity.x;
			}

			if(object.position.x > Display::getWidth() - 8)
			{
				object.position.x = (Display::getWidth() - 8);
				object.velocity.x = -object.velocity.x;
			}

			if(gravityEnabled)
			{
				// If gravity is enabled, gradually have the object come to a halt
				// This would be easier to understand with a diagram
				if(object.position.y < 0)
				{
					object.position.y = 0;

					if(object.velocity.y > RestitutionThreshold)
						object.velocity.y = -object.velocity.y * CoefficientOfRestitution;
					else
						object.velocity.y = 0;
				}
				if(object.position.y > Display::getHeight() - 8)
				{
					object.position.y = (Display::getHeight() - 8);

					if(object.velocity.y > RestitutionThreshold)
						object.velocity.y = -object.velocity.y * CoefficientOfRestitution;
					else
						object.velocity.y = 0;
				}
			}
			else
			{
				// If gravity isn't enabled, bounce off the y sides as well
				if(object.position.y < 0)
				{
					object.position.y = 0;
					object.velocity.y = -object.velocity.y;
				}

				if(object.position.y > Display::getHeight() - 8)
				{
					object.position.y = (Display::getHeight() - 8);
					object.velocity.y = -object.velocity.y;
				}
			}

			// Finally, update position using velocity
			object.position += object.velocity;
		}
	}
};

// Needed here to shut Code::Blocks up when compiling for the Pokitto Simulator
constexpr Number Game::CoefficientOfFriction;
constexpr Number Game::CoefficientOfGravity;
constexpr Number Game::CoefficientOfRestitution;
constexpr Number Game::RestitutionThreshold;
constexpr Number Game::InputForce;
