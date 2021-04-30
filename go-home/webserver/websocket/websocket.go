package homewebsocket

import (
	"log"
	"net/http"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/gorilla/websocket"
)

type TestJson struct {
	Data    string `json:"data"`
	Isotime string `json:"isotime"`
}

type connection struct {
	websct *websocket.Conn
	send   chan TestJson
}
type SensorSubscribe struct {
	SensorConn *connection
	SensorId   string
}
type SensorData struct {
	Data     TestJson
	SensorId string
}
type Sensors struct {
	Topic      map[string]map[*connection]bool
	Register   chan SensorSubscribe
	Unregister chan SensorSubscribe
	Broadcast  chan SensorData
}

var H = Sensors{
	Topic:      make(map[string]map[*connection]bool),
	Register:   make(chan SensorSubscribe),
	Unregister: make(chan SensorSubscribe),
	Broadcast:  make(chan SensorData),
}
var upgrader = websocket.Upgrader{
	ReadBufferSize:  1024,
	WriteBufferSize: 1024,
}

func ServeWs(c *gin.Context) {
	upgrader.CheckOrigin = func(c *http.Request) bool { return true }
	ws, err := upgrader.Upgrade(c.Writer, c.Request, nil)
	if err != nil {
		log.Println(err.Error())
		return
	}

	conn := &connection{websct: ws, send: make(chan TestJson)}
	subscribe := SensorSubscribe{SensorConn: conn, SensorId: "sensor"}
	H.Register <- subscribe
	go subscribe.reader()
	go subscribe.writer()

}

func (s SensorSubscribe) reader() {
	var msg TestJson
	log.Println("Reader:", s)
	conn := s.SensorConn.websct
	defer func() {
		H.Unregister <- s
		conn.Close()
	}()
	//conn.SetReadDeadline(time.Now().Add(10))
	//conn.SetPongHandler(func(string) error { conn.SetReadDeadline(time.Now().Add(10)); return nil })

	for {
		err := conn.ReadJSON(&msg)
		defer func() {
			H.Unregister <- s
			s.SensorConn.websct.Close()
		}()
		if err != nil {
			log.Println(err.Error())
			break
		}
		log.Println(msg)
		m := SensorData{
			Data:     msg,
			SensorId: s.SensorId,
		}
		H.Broadcast <- m

	}
}

func (s SensorSubscribe) writer() {
	conn := s.SensorConn
	//	ticker := time.NewTicker(10)
	defer func() {
		//ticker.Stop()
		conn.websct.Close()
	}()
	log.Println("Write obj:", s)
	for {
		select {
		case msg, ok := <-conn.send:
			if !ok {
				err := conn.write(websocket.CloseMessage, TestJson{})
				if err != nil {
					//ticker.Stop()
					conn.websct.Close()
					return
				}
			}
			if err := conn.write(websocket.TextMessage, msg); err != nil {
				return
			}
			/*case <-ticker.C:
			if err := conn.write(websocket.PingMessage, TestJson{Data: "ticker"}); err != nil {
				return
			}*/

		}
	}
}

func (c *connection) write(mt int, payload TestJson) error {
	log.Println("write mt:", mt)
	return c.websct.WriteJSON(payload)
}
func (h *Sensors) Run() {
	for {
		select {
		case s := <-h.Register:
			connections := h.Topic[s.SensorId]
			if connections == nil {
				connections = make(map[*connection]bool)
				h.Topic[s.SensorId] = connections
			}
			h.Topic[s.SensorId][s.SensorConn] = true
		case s := <-h.Unregister:
			connections := h.Topic[s.SensorId]
			if connections != nil {
				if _, ok := connections[s.SensorConn]; ok {
					log.Println(connections)
					delete(connections, s.SensorConn)
					close(s.SensorConn.send)
					if len(connections) == 0 {
						delete(h.Topic, s.SensorId)
					}
				}
			}
		case m := <-h.Broadcast:
			connections := h.Topic[m.SensorId]
			for c := range connections {
				select {
				case c.send <- TestJson{Data: "HEllo from server", Isotime: time.Now().Format(time.RFC3339)}:
				default:
					close(c.send)
					log.Println(connections)
					delete(connections, c)
					if len(connections) == 0 {
						delete(h.Topic, m.SensorId)
					}
				}
			}
		}
	}
}
