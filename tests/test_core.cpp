#include <cassert>
#include <iostream>
#include "../src/core/Types.h"
#include "../src/core/Stroke.h"
#include "../src/core/Page.h"
#include "../src/core/Notebook.h"

using namespace Inkwell;

void testStrokeCreation() {
    Stroke s(ToolType::Pen);
    s.setColor(QColor("#2563EB"));
    s.setBaseWidth(3.0);
    s.addPoint(10.0, 10.0, 0.5, 100);
    s.addPoint(20.0, 30.0, 0.8, 110);
    s.addPoint(40.0, 50.0, 0.6, 120);

    assert(s.pointCount() == 3);
    assert(!s.isEmpty());
    assert(s.containsPoint(QPointF(20.0, 30.0), 5.0));

    // Test JSON round-trip
    QJsonObject json = s.toJson();
    Stroke deserialized = Stroke::fromJson(json);
    assert(deserialized.id() == s.id());
    assert(deserialized.pointCount() == 3);
    assert(deserialized.color() == s.color());

    std::cout << "[PASS] StrokeCreation & Serialization" << std::endl;
}

void testPageAndNotebook() {
    Notebook nb("Math 201");
    Page p(PageSize::A4);

    Stroke s(ToolType::Highlighter);
    s.addPoint(50, 50, 1.0, 0);
    s.addPoint(150, 50, 1.0, 10);
    p.addStroke(s);

    assert(p.strokeCount() == 1);
    nb.addPage(p);
    assert(nb.pageCount() == 1);

    // Serialization round-trip
    QJsonObject nbJson = nb.toJson();
    Notebook restored = Notebook::fromJson(nbJson);
    assert(restored.title() == "Math 201");
    assert(restored.pageCount() == 1);
    assert(restored.page(0)->strokeCount() == 1);

    std::cout << "[PASS] Page & Notebook Management" << std::endl;
}

int main() {
    std::cout << "Running Inkwell Core Engine Unit Tests..." << std::endl;
    testStrokeCreation();
    testPageAndNotebook();
    std::cout << "All Unit Tests Passed Successfully!" << std::endl;
    return 0;
}
