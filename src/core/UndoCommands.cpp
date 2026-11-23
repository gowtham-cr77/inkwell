#include "UndoCommands.h"

namespace Inkwell {

// AddStrokeCommand
AddStrokeCommand::AddStrokeCommand(Page *page, const Stroke &stroke, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_page(page)
    , m_stroke(stroke)
{
    setText("Add Stroke");
}

void AddStrokeCommand::undo()
{
    if (m_page) {
        m_page->removeStroke(m_stroke.id());
    }
}

void AddStrokeCommand::redo()
{
    if (m_page) {
        m_page->addStroke(m_stroke);
    }
}

// RemoveStrokeCommand
RemoveStrokeCommand::RemoveStrokeCommand(Page *page, const QString &strokeId, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_page(page)
    , m_strokeId(strokeId)
{
    setText("Erase Stroke");
    if (m_page) {
        Stroke *s = m_page->strokeById(strokeId);
        if (s) {
            m_stroke = *s;
        }
    }
}

void RemoveStrokeCommand::undo()
{
    if (m_page && !m_stroke.isEmpty()) {
        m_page->addStroke(m_stroke);
    }
}

void RemoveStrokeCommand::redo()
{
    if (m_page) {
        m_page->removeStroke(m_strokeId);
    }
}

// MoveStrokesCommand
MoveStrokesCommand::MoveStrokesCommand(Page *page, const QStringList &strokeIds, const QPointF &delta, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_page(page)
    , m_strokeIds(strokeIds)
    , m_delta(delta)
{
    setText("Move Strokes");
}

void MoveStrokesCommand::undo()
{
    if (!m_page) return;
    for (const auto &id : m_strokeIds) {
        Stroke *s = m_page->strokeById(id);
        if (s) {
            s->translate(-m_delta);
        }
    }
}

void MoveStrokesCommand::redo()
{
    if (!m_page) return;
    for (const auto &id : m_strokeIds) {
        Stroke *s = m_page->strokeById(id);
        if (s) {
            s->translate(m_delta);
        }
    }
}

bool MoveStrokesCommand::mergeWith(const QUndoCommand *other)
{
    if (other->id() != id()) return false;
    const auto *moveCmd = static_cast<const MoveStrokesCommand*>(other);
    if (m_page != moveCmd->m_page || m_strokeIds != moveCmd->m_strokeIds) return false;

    m_delta += moveCmd->m_delta;
    return true;
}

// CallbackCommand
CallbackCommand::CallbackCommand(std::function<void()> doFunc, std::function<void()> undoFunc,
                                 const QString &text, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_doFunc(std::move(doFunc))
    , m_undoFunc(std::move(undoFunc))
{
    setText(text);
}

void CallbackCommand::undo()
{
    if (m_undoFunc) {
        m_undoFunc();
    }
}

void CallbackCommand::redo()
{
    if (m_doFunc) {
        m_doFunc();
    }
}

} // namespace Inkwell
