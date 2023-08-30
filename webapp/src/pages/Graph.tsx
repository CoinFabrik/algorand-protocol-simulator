import { useEffect, useRef, useState } from 'react'
import * as d3 from 'd3'
import data from './data.json'

function Graph(): JSX.Element {

  const svgRef = useRef<SVGSVGElement>(null)

  const [state, setState] = useState(true);


  useEffect (() => {
    const width = 928
    const height = 600

    const color = d3.scaleOrdinal(d3.schemeCategory10);

    const links = data.links.map(d => ({...d}));
    const nodes = data.nodes.map(d => ({...d}));

    const svg = d3.create("svg")
    .attr("width", width)
    .attr("height", height)
    .attr("viewBox", [0, 0, width, height])
    .attr("style", "max-width: 100%; height: auto;");

    const link = svg.append("g")
      .attr("stroke", "#999")
      .attr("stroke-opacity", 0.6)
      .selectAll()
      .data(links)
      .join("line")
      .attr("stroke-width", (d: any) => Math.sqrt(d.value));

    const node = svg.append("g")
      .attr("stroke", "#fff")
      .attr("stroke-width", 1.5)
      .selectAll()
      .data(nodes)
      .join("circle")
      .attr("r", 5)
      .attr("fill", (d: any) => color(d.group));

    node.append("title")
      .text((d: any) => d.id);

    // Add a drag behavior.
    node.call(d3.drag()
      .on("start", dragstarted)
      .on("drag", dragged)
      .on("end", dragended));

    const simulation = d3.forceSimulation(nodes)
      .force("link", d3.forceLink(links).id((d: any) => d.id))
      .force("charge", d3.forceManyBody())
      .force("center", d3.forceCenter(width / 2, height / 2))
      .on("tick", ticked());

    // Set the position attributes of links and nodes each time the simulation ticks.
    function ticked() { 
      link
        .attr("x1", (d: any) => d.source.x)
        .attr("y1", (d: any) => d.source.y)
        .attr("x2", (d: any) => d.target.x)
        .attr("y2", (d: any) => d.target.y);

      node
        .attr("cx", (d: any) => d.x)
        .attr("cy", (d: any) => d.y);
    }

    // Reheat the simulation when drag starts, and fix the subject position.
    function dragstarted(event: any) {
      if (!event.active) simulation.alphaTarget(0.3).restart();
      event.subject.fx = event.subject.x;
      event.subject.fy = event.subject.y;
    }

    // Update the subject (dragged node) position during drag.
    function dragged(event: any) {
      event.subject.fx = event.x;
      event.subject.fy = event.y;
    }

    // Restore the target alpha so the simulation cools after dragging ends.
    // Unfix the subject position now that it’s no longer being dragged.
    function dragended(event: any) {
      if (!event.active) simulation.alphaTarget(0);
      event.subject.fx = null;
      event.subject.fy = null;
    }

    // invalidation.then(() => simulation.stop());

    // return svg.node();

    svgRef.current?.appendChild(svg.node() as Node)

    console.log('hola')

  }, [state])

  return (
    <div>
      <svg ref={svgRef} onClick={() => setState(!state)}/>
    </div>
  )
}

export default Graph