
/* from https://bl.ocks.org/mbostock/3883245 */



var parseTime = d3.timeParse("%Y-%m-%e %H:%M:%S");  //"2018-7-2 14:44:59"

// parse a CSV string (a long string of newline-separated 
// lines of comma-separated values, with a header as D3
// expects) into an array of objects {date:.., pm25:...}
function parserawdata(csvstring) {
  var parsed = d3.csvParse(csvstring, function(d) {
    d.date = parseTime(d.datetime);
    d.pm25 = +d.pm25;
    console.log(" " + d.date + " " + d.pm25);
    return d;
    });
  return parsed;
}

// given an array of objects {date:, pm25:}, make a graph in the
// single svg element on the page.  Sorry yes, I know it's assuming
// an awful lot of infrastructure so far, e.g. the graph labels
function graphparseddata(data, svgid) {
  var svg = d3.select(svgid),
        margin = {top: 20, right: 20, bottom: 30, left: 50},
        width = +svg.attr("width") - margin.left - margin.right,
        height = +svg.attr("height") - margin.top - margin.bottom,
        g = svg.append("g").attr("transform", "translate(" + margin.left + "," + margin.top + ")");

  var x = d3.scaleTime()
        .rangeRound([0, width]);

  var y = d3.scaleLinear()
        .rangeRound([height, 0]);

  var line = d3.line()
        .x(function(d) { return x(d.date); })
        .y(function(d) { return y(d.pm25); });

  x.domain(d3.extent(data, function(d) { return d.date; }));
  y.domain(d3.extent(data, function(d) { return d.pm25; }));

  g.append("g")
      .attr("transform", "translate(0," + height + ")")
      .call(d3.axisBottom(x))
    .select(".domain")
      .remove();

  g.append("g")
      .call(d3.axisLeft(y))
    .append("text")
      .attr("fill", "#000")
      .attr("transform", "rotate(-90)")
      .attr("y", 6)
      .attr("dy", "0.71em")
      .attr("text-anchor", "end")
      .text("uncalibrated PM2.5 ug/m3");

  g.append("path")
      .datum(data)
      .attr("fill", "none")
      .attr("stroke", "steelblue")
      .attr("stroke-linejoin", "round")
      .attr("stroke-linecap", "round")
      .attr("stroke-width", 1.5)
      .attr("d", line);
}