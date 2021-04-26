package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"strconv"
)

const Api_url string = "http://egemeric.gen.tr:8080/api/post/updatehomeinfo"

type ReqUrls struct {
	AbsPath     string
	InsideHum   string
	OutsideHum  string
	InsideTemp  string
	OutsideTemp string
}
type HomInfo struct {
	OutsideTemp float64 `json:"outsidetemp"`
	OutsideHum  uint    `json:"outsidehum" `
	InsideTemp  float64 `json:"insidetemp" `
	InsideHum   uint    `json:"insidehum" `
	Latency     float64 `json:"latency"`
}
type UpdateRequest struct {
	Data HomInfo `json:"data" `
	User string  `json:"user" `
	Key  string  `json:"key" `
}

var Urls ReqUrls = ReqUrls{AbsPath: "http://10.1.1.112", InsideHum: "/get_hum_in", OutsideHum: "/get_hum_out", InsideTemp: "/get_temp_in", OutsideTemp: "/get_temp_out"}

func get_sensor_data(data_type_url string) string {
	resp, err := http.Get(Urls.AbsPath + data_type_url)
	if err != nil {
		log.Fatalln(err)
	}
	body, err := ioutil.ReadAll(resp.Body)

	if err != nil {
		log.Fatalln(err)
	}
	fmt.Println(string(body))
	return string(body)
}
func send_update_request(req_data UpdateRequest) (e error) {
	payloadBuf := new(bytes.Buffer)
	json.NewEncoder(payloadBuf).Encode(req_data)
	req, _ := http.NewRequest("POST", Api_url, payloadBuf)
	client := &http.Client{}
	res, e := client.Do(req)
	if e != nil {
		return e
	}
	defer res.Body.Close()
	return nil
}

func main() {
	var sensor_data HomInfo
	in_tmp, _ := strconv.ParseFloat(get_sensor_data(Urls.InsideTemp), 64)
	out_tmp, _ := strconv.ParseFloat(get_sensor_data(Urls.OutsideTemp), 64)
	in_hum, _ := strconv.ParseFloat(get_sensor_data(Urls.InsideHum), 64)
	out_hum, _ := strconv.ParseFloat(get_sensor_data(Urls.OutsideHum), 64)
	sensor_data = HomInfo{InsideTemp: in_tmp, OutsideTemp: out_tmp, InsideHum: uint((in_hum)), OutsideHum: uint(out_hum)}
	update_req := UpdateRequest{Data: sensor_data, User: "apiuser", Key: "api-user-key"}
	err := send_update_request(update_req)
	if err != nil {
		log.Fatalln("Connection Error", err)
	}

}
