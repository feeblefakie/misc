package main

import (
	"flag"
	"html/template"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"sync"

	"github.com/feeblefakie/misc/golang/trace"
	"github.com/stretchr/gomniauth"
	"github.com/stretchr/gomniauth/providers/google"
	"github.com/stretchr/objx"
)

const (
	ClientID          = "1096523185743-5as521ptb79hlji5c2d00nudjo8qmnrg.apps.googleusercontent.com"
	ClientSecret      = "o4TUHcyGsFjHhYh_UQetL5E7"
	GoogleCallbackURL = "http://localhost:8080/auth/callback/google"
)

type templateHandler struct {
	once     sync.Once
	filename string
	templ    *template.Template
}

func (t *templateHandler) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	t.once.Do(func() {
		t.templ = template.Must(template.ParseFiles(filepath.Join("templates", t.filename)))
	})

	data := map[string]interface{}{
		"Host": r.Host,
	}
	if authCookie, err := r.Cookie("auth"); err == nil {
		data["UserData"] = objx.MustFromBase64(authCookie.Value)
	}

	t.templ.Execute(w, data)
}

func main() {
	var addr = flag.String("addr", ":8080", "applications's address")
	flag.Parse()

	gomniauth.SetSecurityKey("some security key")
	gomniauth.WithProviders(
		google.New(ClientID, ClientSecret, GoogleCallbackURL),
	)

	r := newRoom()
	r.tracer = trace.New(os.Stdout)

	http.Handle("/", MustAuth(&templateHandler{filename: "chat.html"}))
	http.Handle("/login", &templateHandler{filename: "login.html"})
	http.HandleFunc("/auth/", loginHandler)
	http.Handle("/room", r)

	go r.run()

	if err := http.ListenAndServe(*addr, nil); err != nil {
		log.Fatal("ListenAndServe:", err)
	}
}
