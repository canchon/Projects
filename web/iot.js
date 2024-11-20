const fetchData = (module, accion, parameters) => {
    return new Promise((resolve, reject) => {
        const url = "https://api.thingspeak.com/channels/2754061/feeds.json?api_key=I07CKSIW92TBS2PI&days=30"
        const json = {
            'module': module,
            'accion': accion,
            'parameters': parameters,
        }
        const data = new FormData()
        data.append('json', JSON.stringify(json))
        const req = new XMLHttpRequest()
        req.open("get", url)
        req.onload = () => {
            (req.status === 200)
                ? resolve(req.responseText)
                : reject(req.error)
        }
        req.send()
    })
}

const bringData = async () => {
    try {
        let response = await fetchData("monitoreoBioGas", "getValues", "{}")
        response = JSON.parse(response)
        console.log(response)
        let i = 0
        const rowTable = document.getElementById("row_table")
        rowTable.innerHTML = ""

        for (const e of response.feeds.reverse()) {
            i++
            rowTable.innerHTML += `
            <!--<tr >
                <th scope="row">${i}</th>
                <td>${e.temperature}</td>
                <td>${e.humidity}</td>
                <td>${e.date}</td>
            </tr>-->


            <tr>
                <th scope="row">${i}</th>
                <td scope="col">${e.field1}</td>
                <td scope="col">${e.field2}</td>
                <td scope="col">${formatDate(e.created_at)}</td>
            </tr>
            `
        }
    } catch (error) { console.error(error) }
}

const formatDate = (DateResponse) => {
    let newDate = new Date(DateResponse)
    newDate.setHours(newDate.getHours() - 0)
    return newDate.toLocaleString()
}

bringData()