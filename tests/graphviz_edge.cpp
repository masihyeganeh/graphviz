#include <string>

#include "graphviz_edge.h"
#include "graphviz_node.h"
#include "svg_element.h"

GraphvizEdge::GraphvizEdge(SVG::SVGElement &svg_g_element)
    : m_edgeop(svg_g_element.graphviz_id), m_svg_g_element(svg_g_element) {}

void GraphvizEdge::add_bbox() { m_svg_g_element.add_bbox(); }

void GraphvizEdge::add_outline_bbox() { m_svg_g_element.add_outline_bbox(); }

void GraphvizEdge::add_outline_overlap_bbox(const GraphvizNode &node,
                                            const double tolerance) {
  m_svg_g_element.add_outline_overlap_bbox(node.svg_g_element(), tolerance);
}

std::string_view GraphvizEdge::edgeop() const { return m_edgeop; }

std::string GraphvizEdge::fillcolor() const {
  const auto fill = m_svg_g_element.attribute_from_subtree<std::string>(
      &SVG::SVGAttributes::fill, &SVG::SVGElement::is_closed_shape_element, "");
  const auto fill_opacity = m_svg_g_element.attribute_from_subtree<double>(
      &SVG::SVGAttributes::fill_opacity,
      &SVG::SVGElement::is_closed_shape_element, 1);
  if (fill.empty() && fill_opacity == 1) {
    return "";
  }
  return SVG::to_dot_color(fill, fill_opacity);
}

const SVG::SVGElement &GraphvizEdge::svg_g_element() const {
  return m_svg_g_element;
}

SVG::SVGRect GraphvizEdge::bbox() const { return m_svg_g_element.bbox(); }

SVG::SVGPoint GraphvizEdge::center() const { return bbox().center(); }

std::string GraphvizEdge::color() const {
  const auto stroke = m_svg_g_element.attribute_from_subtree<std::string>(
      &SVG::SVGAttributes::stroke, &SVG::SVGElement::is_shape_element, "");
  const auto stroke_opacity = m_svg_g_element.attribute_from_subtree<double>(
      &SVG::SVGAttributes::stroke_opacity, &SVG::SVGElement::is_shape_element,
      1);
  return SVG::to_dot_color(stroke, stroke_opacity);
}

double GraphvizEdge::penwidth() const {
  return m_svg_g_element.attribute_from_subtree<double>(
      &SVG::SVGAttributes::stroke_width, &SVG::SVGElement::is_shape_element, 1);
}

SVG::SVGRect GraphvizEdge::outline_bbox() const {
  return m_svg_g_element.outline_bbox();
}