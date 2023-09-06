import { useEffect, useRef } from 'react'
import * as d3 from 'd3'
import data from './data.json'

function Graph(): JSX.Element {

  const svgRef = useRef(null)

  const width = window.innerWidth*0.9
  const height = window.innerHeight*0.9

  const nodes = data.nodes.map((node: any) => {
    return {
      ...node,
      x: Math.random() * 800,
      y: Math.random() * 800
    }
  })

  const links = data.links.map((link: any) => {
    return {
      ...link,
      source: link.source,
      target: link.target
    }
  })

  useEffect(() => {

    const margin = { top: 20, right: width/2, bottom: 20, left: width/2 }

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
      .attr('fill', (d: any) => {
        if (d.type === 'participation') {
          return 'red'
        } else {
          return 'blue'
        }
      })

    console.log(nodes)

    node.append('title')
      .text((d: any) => `${d.id} - ${d.type}`)

    simulation.on('tick', (): any => {
      link
        .attr('x1', (d: any) => d.source.x)
        .attr('y1', (d: any) => d.source.y) 
        .attr('x2', (d: any) => d.target.x)
        .attr('y2', (d: any) => d.target.y)

      node
        .attr('cx', (d: any) => d.x)
        .attr('cy', (d: any) => d.y)
    })

    const drag = (simulation: any): any => {
        
      function dragstarted(event: any, d: any) {
        if (!event.active) simulation.alphaTarget(0.3).restart()
        d.fx = d.x
        d.fy = d.y
      }
        
      function dragged(event: any, d: any) {
        d.fx = event.x
        d.fy = event.y
      }
        
      function dragended(event: any, d: any) {
        if (!event.active) simulation.alphaTarget(0)
        d.fx = null
        d.fy = null
      }
        
      return d3.drag()
        .on('start', dragstarted)
        .on('drag', dragged)
        .on('end', dragended)
    }

    node.call(drag(simulation))
    
  }, [])

  return (
    <div>
      <h1 className='text-center mt-6 text-2xl font-bold'>Nodes Graph</h1>
      <svg ref={svgRef} width={width} height={height} />
    </div>
  )
}

export default Graph