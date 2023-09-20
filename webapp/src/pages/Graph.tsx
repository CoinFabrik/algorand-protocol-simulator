import { useEffect, useRef, useContext, useState } from 'react'
import * as d3 from 'd3'
import Log from '../assets/out.ts'
import Context from '../context/Context'

function Graph(): JSX.Element {

  const context  = useContext(Context)

  const data = context.context

  //[INFO]	S 1546 3 0 2 10.200000286101 6.0170131

  const [rounds, setRounds] = useState<any[]>([])

  /* Metrica real aproximada de nodos */

  // 300 Participation
  // 120 Relays

  /* out.log */

  // S 340 4 0 1 13.600000381468 8.9779329

  // S: Step Event
  // 340: ID del nodo
  // 4: Round
  // 0: Periodo (otra vuelta del protocolo cuando no confirmo el bloque)
  // 1: Status
  // 13.600000381468: Tiempo de simulacion
  // 8.9779329: Tiempo cronologico

  // G 2 405 3.400000095367 2.5861584
  
  // G: Global - ningun nodo en particular
  // 2: Current global round
  // 405: ID del bloque que genero ese cambio (hash del bloque)
  // 3.400000095367: Tiempo de simulacion
  // 2.5861584: Tiempo cronologico


  // console.log(data)

  const svgRef = useRef(null)

  const width = window.innerWidth*0.5
  const height = window.innerHeight*0.8

  const nodes = data['nodes'].map((node: any) => {
    return {
      ...node,
      x: Math.random() * 800,
      y: Math.random() * 800
    }
  })

  const links = data['links'].map((link: any) => {
    return {
      ...link,
      source: link.source,
      target: link.target
    }
  })


  const outLog: any = []

  async function logSplit () {
    await Log.split('\n').forEach((line: any) => {
      if(line.includes('G') && line.includes('[INFO]')) {
        const lineSplit = line.split(' ')
        const round = lineSplit[1]
        const block = lineSplit[2]
        const simulationTime = lineSplit[3]
        const realTime = lineSplit[4]
        outLog.push({
          round,
          block,
          simulationTime,
          realTime
        })
      }
    })
  }

  

  useEffect(() => {

    logSplit()

    setRounds(outLog)

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
      .force('charge', d3.forceManyBody().strength(-8))
      .force('center', d3.forceCenter(innerWidth/2, innerHeight/2))
      // .force('x', d3.forceX().strength(0.01))
      // .force('y', d3.forceY().strength(0.02))

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

    // console.log(nodes.map((node: any) => node.type))

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
        if (!event.active) simulation.alphaTarget(0.5).restart()
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

    console.log(rounds)
    
  }, [rounds.length, data])

  return (
    <div>
      <h1 className='text-center mt-6 text-4xl font-bold'>Nodes Graph</h1>
      <h1 className='text-center mb-8 text-2xl font-light'>With data log</h1>
      <div className='inline-flex gap-20 ml-20'>
      <svg ref={svgRef} width={width} height={height} />
      <table className='table-auto h-10'>
        <thead>
          <tr>
            <th className='px-4 py-2'>Round</th>
            <th className='px-4 py-2'>Block</th>
            <th className='px-4 py-2'>Simulation Time</th>
            <th className='px-4 py-2'>Real Time</th>
          </tr>
        </thead>
        <tbody>
          {rounds.map((round: any, index: number) => (
            <tr key={index}>
              <td className='border px-4 py-2'>{round.round}</td>
              <td className='border px-4 py-2'>{round.block}</td>
              <td className='border px-4 py-2'>{Math.round(round.simulationTime * 10) / 10}s</td>
              <td className='border px-4 py-2'>{Math.round(round.realTime * 100) / 100}s</td>
            </tr>
          ))}
        </tbody>
      </table>
      </div>
    </div>
  )
}

export default Graph