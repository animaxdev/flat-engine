#ifndef FLAT_STATE_MACHINE_H
#define FLAT_STATE_MACHINE_H

#include <memory>

namespace flat
{
namespace state
{

class Agent;
class State;

class Machine
{
	public:
		Machine(Agent& agent);
		~Machine();

		void setState(State* state);

		inline State* getState() { return m_currentState.get(); }
		inline const State* getState() const { return m_currentState.get(); }

		void update();

	private:
		std::unique_ptr<State> m_currentState;
		Agent& m_agent;
};

} // state
} // flat

#endif // FLAT_STATE_MACHINE_H


