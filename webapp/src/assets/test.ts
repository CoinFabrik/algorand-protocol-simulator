
  // useEffect(() => {
  //   // Setting up svg
  //   const w = 400
  //   const h = 400
  //   const svg = d3.select(svgRef.current)
  //     .attr('width', w)
  //     .attr('height', h)
  //     .style('background-color', '#d3d3d3')
  //     .style('margin-top', '50')
  //     .style('margin-left', 'auto')
  //     .style('margin-right', 'auto')
  //     .style('overflow', 'visible')

  //   // Setting the scaling
  //   const xScale = d3.scaleLinear()
  //     .domain([0, data.length - 1])
  //     .range([0, w])
  //   const yScale = d3.scaleLinear()
  //     .domain([0, h])
  //     .range([h, 0])
  //   const generateScaledLine = d3.line<number>()
  //     .x((d: any, i: any) => xScale(i))
  //     .y(yScale)
  //     .curve(d3.curveCardinal)

  //   // Setting the axis
  //   const xAxis = d3.axisBottom(xScale)
  //     .ticks (data.length)
  //     .tickFormat((index: any) => index + 1)
  //   const yAxis = d3.axisLeft(yScale)
  //     .ticks(5)
  //   svg.append('g')
  //     .call(xAxis)
  //     .attr('transform', `translate(0, ${h})`)
  //   svg.append('g')
  //     .call(yAxis)

  //   // Setting up the data for the svg
  //   svg.selectAll('.line')
  //     .data([data])
  //     .join('path')
  //     .attr('d', (d: any) => generateScaledLine(d) as string)
  //     .attr('fill', 'none')
  //     .attr('stroke', 'black')
  // }, [data])