import { useRef } from 'react'
import * as d3 from 'd3'
import data from './data.json'

function Graph(): JSX.Element {

  const svgRef = useRef(null)

  const nodes = data.nodes.map((node: any) => {
    return {
      ...node,
      x: Math.random() * 900,
      y: Math.random() * 900
    }
  })

  const links = data.links.map((link: any) => {
    return {
      ...link,
      source: link.source,
      target: link.target
    }
  })

  const width = 600
  const height = 600

  const margin = { top: 20, right: 20, bottom: 20, left: 20 }

  const innerWidth = width - margin.left - margin.right
  const innerHeight = height - margin.top - margin.bottom

  const svg = d3.select(svgRef.current)
    .attr('width', width)
    .attr('height', height)

  const g = svg.append('g')
    .attr('transform', `translate(${margin.left}, ${margin.top})`)
    .attr('width', innerWidth)
    .attr('height', innerHeight)

  const simulation = d3.forceSimulation(nodes)
    .force('link', d3.forceLink(links).id((d: any) => d.id))
    .force('charge', d3.forceManyBody())
    .force('center', d3.forceCenter(innerWidth / 2, innerHeight / 2))

  const link = g.append('g')
    .attr('stroke', '#999')
    .attr('stroke-opacity', 0.6)
    .selectAll('line')
    .data(links)
    .join('line')
    .attr('stroke-width', (d: any) => Math.sqrt(d.value))

  const node = g.append('g')
    .attr('stroke', '#fff')
    .attr('stroke-width', 1.5)
    .selectAll('circle')
    .data(nodes)
    .join('circle')
    .attr('r', 5)
    .attr('fill', '#000')

  node.append('title')
    .text((d: any) => d.id)

  simulation.on('tick', () => {
    link
      .attr('x1', (d: any) => d.source.x)
      .attr('y1', (d: any) => d.source.y) 
      .attr('x2', (d: any) => d.target.x)
      .attr('y2', (d: any) => d.target.y)

    node
      .attr('cx', (d: any) => d.x)
      .attr('cy', (d: any) => d.y)
  })

  return (
    <div>
      <svg ref={svgRef} width={width} height={height} />
    </div>
  )
}

export default Graph