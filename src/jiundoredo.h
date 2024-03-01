#ifndef JIUNDOREDO_H
#define JIUNDOREDO_H
#include <QList>
#include <stdexcept>
//! \todo Add "monitor/changed" function pointer
//! \todo Reset should accept default value
template <typename T> class JIUndoRedo
{
public:
    JIUndoRedo(const T& initialHead)
    {
        reset(initialHead);
    }
    void push(const T& item)
    {
        if (m_stackPosition>=0 && m_stackPosition<(stackSize()-1))
        {
            m_stack.erase(m_stack.begin()+m_stackPosition+1,m_stack.end());
        }
        m_stack.push_back(item);
        ++m_stackPosition;
    }
    const T& head() const
    {
        if (m_stackPosition>=stackSize()) {
            throw std::out_of_range(
                QString("head out of range (stack pos: %1, stack size: %2)")
                    .arg(m_stackPosition).arg(stackSize()).toStdString());
        }
        return m_stack.at(m_stackPosition);
    }
    int stackSize() const
    {
        return m_stack.size();
    }
    int stackPosition() const
    {
        return m_stackPosition;
    }
    bool canUndo() const
    {
        return m_stackPosition>0;
    }
    bool canRedo() const
    {
        return (m_stackPosition+1)<stackSize();
    }
    bool undo()
    {
        if (!canUndo())
        {
            return false;
        }
        --m_stackPosition;
        return true;
    }
    bool redo()
    {
        if (!canRedo())
        {
            return false;
        }
        ++m_stackPosition;
        return true;
    }

    void    reset(const T& initialHead)
    {
        m_initialHead = initialHead;
        m_stack.clear();
        m_stackPosition=-1;
        push(m_initialHead);
    }
private:
    T m_initialHead;
    QList<T> m_stack;
    int m_stackPosition=-1;

};
#endif // JIUNDOREDO_H
