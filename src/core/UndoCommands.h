#pragma once

#include "Stroke.h"
#include "Page.h"
#include <QUndoCommand>
#include <functional>

namespace Inkwell {

class AddStrokeCommand : public QUndoCommand {
public:
    AddStrokeCommand(Page *page, const Stroke &stroke, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
private:
    Page *m_page;
    Stroke m_stroke;
};

class RemoveStrokeCommand : public QUndoCommand {
public:
    RemoveStrokeCommand(Page *page, const QString &strokeId, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
private:
    Page *m_page;
    QString m_strokeId;
    Stroke m_stroke; // saved for undo
};

class MoveStrokesCommand : public QUndoCommand {
public:
    MoveStrokesCommand(Page *page, const QStringList &strokeIds, const QPointF &delta, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
    int id() const override { return 1001; }
    bool mergeWith(const QUndoCommand *other) override;
private:
    Page *m_page;
    QStringList m_strokeIds;
    QPointF m_delta;
};

// Generic callback-based undo command for flexibility
class CallbackCommand : public QUndoCommand {
public:
    CallbackCommand(std::function<void()> doFunc, std::function<void()> undoFunc,
                    const QString &text, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
private:
    std::function<void()> m_doFunc;
    std::function<void()> m_undoFunc;
};

} // namespace Inkwell
