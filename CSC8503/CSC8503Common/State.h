#pragma once

#include <functional>
#include <string>

namespace NCL {
	namespace CSC8503 {
		typedef std::function<void(float)> StateUpdateFunction;

		class State	{
		public:
			State() {}
			State(std::string stateName, StateUpdateFunction someFunc) {
				name = stateName;
				func = someFunc;				
			}
			virtual ~State() {}

			void Update(float dt) {
				if (func != nullptr) {
					func(dt);
				}
			}

			std::string GetName() const { return name; }

		protected:
			StateUpdateFunction func;
			std::string name;
		};
	}
}