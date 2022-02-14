const updateInterval = 100;
const getUrl = "http://192.168.4.1:80/";
const setUrl = "http://192.168.4.1:80/";
// const getUrl = "http://localhost:80/";
// const setUrl = "http://localhost:80/";
let chart = false;

let mode = 0;
current = [];
voltage = [];
power = [];
const foo = [{
    yAxisID: "current",
    label: "Strom",
    data: current,
    backgroundColor: "#3498db",
    borderColor: "#3498db",
    pointBackgroundColor: "#3498db",
    pointBorderColor: "#fff",
    tension: 0.4,
  },
  {
    yAxisID: "voltage",
    label: "Spannung",
    data: voltage,
    backgroundColor: "#2ecc71",
    borderColor: "#2ecc70",
    pointBackgroundColor: "#2ecc71",
    pointBorderColor: "#fff",
    tension: 0.4,
  },
  {
    yAxisID: "power",
    label: "Leistung",
    data: power,
    backgroundColor: "#fab900",
    borderColor: "#fab900",
    pointBackgroundColor: "#fab900",
    pointBorderColor: "#fff",
    tension: 0.4,
  },
];
const time = [];

start = () => {
  setInterval(function () {
    getValues();
  }, updateInterval);
};

calcPower = (current, voltage) => {
  return current * voltage;
};

getValues = async () => {
  await fetch(getUrl)
    .then((response) => response.json())
    .then((data) => {
      data.voltage = ((data.voltage * 10 ** -3) / 100 * 1100).toFixed(1);
      data.current = (data.current * 10 ** -3).toFixed(2);
      data.power = calcPower(data.current, data.voltage);
      updateDOM(data);
      fillArrays(data);
    });
};

fillArrays = (data) => {
  if (current.length > 99) {
    current.shift();
  }
  current.push(data.current);

  if (voltage.length > 99) {
    voltage.shift();
  }
  voltage.push(data.voltage);

  if (power.length > 99) {
    power.shift();
  }
  power.push(data.power);

  if (time.length > 99) {
    time.shift();
  }
  time.push(moment().format("HH:mm:ss"));
};

setValues = async (mode, value) => {
  console.log("Mode: ", mode);
  console.log("Value: ", value);
  await fetch(setUrl, {
      method: "POST",
      body: JSON.stringify({
        mode: mode,
        value: Number(value),
      }),
    })
    .then((response) => {
      console.log("Request complete: ", response);
      return response.json();
    })
    .then((data) => {
      console.log(data);
    });
};

updateDOM = (data) => {
  DOMcheckInputs();
  DOMsetValues(data);
  DOMcreateChart();
};

DOMsetValues = (data) => {
  const cur = document.getElementById("value-current");
  const vol = document.getElementById("value-voltage");
  const pow = document.getElementById("value-power");
  cur.innerText = `${data.current} A`;
  vol.innerText = data.voltage + " V";
  pow.innerText = data.power + " W";
};

DOMcreateChart = () => {
  const d = foo.filter((bar) => bar.yAxisID !== mode);
  const config = {
    type: "line",
    data: {
      labels: time,
      datasets: d,
    },
    plugins: [ChartDataLabels],
    options: {
      scales: {
        x: {
          ticks: {
            font: {
              family: "JetBrains Mono",
            },
            color: "#fff",
          },
          display: true,
          position: "bottom",
        },
        [d[0].yAxisID]: {
          ticks: {
            max: Math.max.apply(this, d[0].data) + 2,
            min: Math.min.apply(this, d[0].data) - 2,
            font: {
              family: "JetBrains Mono",
            },
            color: "#fff",
          },
          type: "linear",
          display: true,
          position: "left",
          title: {
            font: {
              family: "JetBrains Mono",
            },
            color: "#fff",
            display: true,
            text: d[0].label,
          },
        },
        [d[1].yAxisID]: {
          ticks: {
            max: Math.max.apply(this, d[1].data),
            min: Math.min.apply(this, d[1].data),
            font: {
              family: "JetBrains Mono",
            },
            color: "#fff",
          },
          type: "linear",
          display: true,
          position: "right",
          title: {
            font: {
              family: "JetBrains Mono",
            },
            color: "#fff",
            display: true,
            text: d[1].label,
          },
          grid: {
            drawOnChartArea: false,
          },
        },
      },
      animation: {
        duration: 0,
      },
      plugins: {
        datalabels: {
          anchor: 'end',
          align: 'end',
          color: '#fff',
          display: function (context) {
            return (context.dataIndex === 99);
          }
        },
        legend: {
          labels: {
            color: "#fff",
            font: {
              family: "JetBrains Mono",
              size: "18px"
            },
          },
        },
      },
      elements: {
        point: {
          radius: 0
        }
      },
    },
  };
  if (!chart) {
    chart = new Chart(document.getElementById("chart"), config);
  } else {
    chart.update();
  }
};

DOMcheckInputs = () => {
  const selector = document.getElementById("constant-selector");
  const value = document.getElementById("value-input");
  selector.onchange = (event) => {
    console.log(event.target.value);
    if (event.target.value === "current") {
      mode = 1;
    } else if (event.target.value === "power") {
      mode = 2;
    } else if (event.target.value === "resistance") {
      mode = 3;
    } else {
      mode = 0;
    }
    chart.destroy();
    chart = 0;
    DOMcreateChart();
  };
  value.onchange = (event) => {
    console.log(event.target.value);
    setValues(mode, event.target.value);
  };
};

start();