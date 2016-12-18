#include "Bool3State.hpp"

Bool3State::Bool3State()
: m_value(UNDEF)
{}

Bool3State::Bool3State(bool value)
: m_value(UNDEF)
{
    SetVal(value);
}

Bool3State::~Bool3State()
{}

void Bool3State::SetVal(bool val)
{
    m_value = val ? TRUE : FALSE;
}
void Bool3State::SetUndef()
{
    m_value = UNDEF;
}
Bool3State::State Bool3State::GetVal() const
{
    return m_value;
}
