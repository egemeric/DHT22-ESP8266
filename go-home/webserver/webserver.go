package homewebserver

import (
	homewebsocket "go-home/webserver/websocket"

	"net/http"

	"github.com/gin-gonic/gin"
)

type Testdata struct {
	Text string `json:"text"`
	Time string `json:"isotime"`
}

var router *gin.Engine

func init() {
	router = gin.Default()
	router.LoadHTMLGlob("webserver/templates/*")
	router.GET("/ws", homewebsocket.ServeWs)
	router.GET("/", home)
}

func home(c *gin.Context) {
	c.HTML(http.StatusOK, "index.html", nil)
}

func StartServer() {
	go homewebsocket.H.Run()
	router.Run(":9000")
}
